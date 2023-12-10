#include <sdk/core/base-src.h>
#include <sdk/core/pad.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;

static logger *logger_ = stdout_color_mt("BaseSource").get();

BaseSource::BaseSource(const string &name, bool is_async) : Element(name) {
  if (!is_async) {
    source_pad_ = new Pad(kPadSource, "src");
    AddPad(source_pad_);
    queue_ = nullptr;
  } else {
    queue_ = new Queue(name + "-queue");
    source_pad_ = queue_->GetSourcePad();
  }
  state_ = kStreamStateStopped;
  stepCount_ = 0;
}

BaseSource::~BaseSource() {
  if (state_ != kStreamStateStopped) {
    Stop();
  }
  if (queue_ != nullptr) {
    delete queue_;
  } else {
    delete source_pad_;
  }
}

void BaseSource::PushFrame(Mat &frame) {}

void BaseSource::GenerateLoop() {
  StreamState state;
  if (InitializeSource() != true) {
    logger_->error("Failed to initialize source");
    return;
  }

  do {
    {
      unique_lock<mutex> lock(mutex_);
      state = state_;
      if (state == kStreamStateStopped) {
        break;
      } else if (state == kStreamStatePaused) {
        // state_, not state, by intention.
        logger_->info("Paused, taking a nap zzz...");
        condvar_.wait(lock, [this] {
          return ((state_ != kStreamStatePaused) || (stepCount_ > 0));
        });
        logger_->info("Rising from under, get back to work!");
      }
    }
    Mat frame = GenerateFrame();
    if (frame.empty()) {
      logger_->error("Failed to generate frame");
      break;
    }

    if (queue_ != nullptr) {
      queue_->PushFrame(frame);
    } else {
      source_pad_->PushFrame(frame);
    }
    if (stepCount_ > 0) stepCount_--;
  } while (state != kStreamStateStopped);
  CleanupSource();

  {
    unique_lock<mutex> lock(mutex_);
    state_ = kStreamStateStopped;
  }
}

Pad *BaseSource::GetSourcePad() { return source_pad_; }

/**
 * @brief state-machine (by asciiflow.com)
 *
 *                ┌───────────┐   pause()
 *       step()┌──►           ◄──────────┐
 *             └──┤  PAUSED   │          │
 *      ┌─────────┤           ├──────┐   │
 *      │         └───────────┘      │   │
 *      │ stop()             resume()│   │
 *      │                            │   │
 *   ┌──▼────────┐   start()    ┌────▼───┴───┐
 *   │           ├──────────────►            │
 *   │  STOPPED  │              │  PLAYING   │
 *   │           ◄──────────────┤            │
 *   └───────────┘ stop()/eof   └────────────┘
 */
bool BaseSource::Start() {
  if (state_ != kStreamStateStopped) {
    logger_->warn("Source is already started.");
    return false;
  }
  {
    // lock on write
    unique_lock<mutex> lock(mutex_);
    state_ = kStreamStatePlaying;
    source_pad_->PushState(state_);
  }

  thread_ = new thread(&BaseSource::GenerateLoop, this);

  return true;
}

bool BaseSource::Stop() {
  if (state_ == kStreamStateStopped) {
    logger_->warn("Source is already stopped.");
    return false;
  }

  bool last_wake = (state_ == kStreamStatePaused);
  {
    unique_lock<mutex> lock(mutex_);
    state_ = kStreamStateStopped;
    source_pad_->PushState(state_);
  }

  if (last_wake) {
    condvar_.notify_one();
  }

  thread_->join();
  delete thread_;
  return true;
}

bool BaseSource::Resume() {
  if (state_ != kStreamStatePaused) {
    logger_->warn("Source is not paused.");
    return false;
  }
  {
    unique_lock<mutex> lock(mutex_);
    state_ = kStreamStatePlaying;
    source_pad_->PushState(state_);
  }
  condvar_.notify_one();
  return true;
}

bool BaseSource::Pause() {
  if (state_ != kStreamStatePlaying) {
    logger_->warn("Source is not playing.");
    return false;
  }
  {
    unique_lock<mutex> lock(mutex_);
    state_ = kStreamStatePaused;
    source_pad_->PushState(state_);
  }
  return true;
}

bool BaseSource::Step() {
  if (state_ != kStreamStatePaused) {
    logger_->warn("Source is not paused.");
    return false;
  }
  stepCount_++;
  condvar_.notify_one();
  return true;
}

StreamState BaseSource::GetState() { return state_; }