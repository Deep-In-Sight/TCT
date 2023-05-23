#include <sdk/core/base-src.h>
#include <sdk/core/pad.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;

static logger *logger_ = stdout_color_mt("BaseSource").get();

BaseSource::BaseSource(const string &name) : Element(name) {
  source_pad_ = new Pad(kPadSource, "src");
  AddPad(source_pad_);
  state_ = kStreamStateStopped;
}

BaseSource::~BaseSource() {
  if (state_ != kStreamStateStopped) {
    Stop();
  }
  delete source_pad_;
}

void BaseSource::PushFrame(Mat &frame) {}

void BaseSource::GenerateLoop() {
  StreamState state;
  // cout << "child address:" << this << endl;
  InitializeSource();
  do {
    {
      unique_lock<mutex> lock(mutex_);
      state = state_;
      if (state == kStreamStateStopped) {
        break;
      } else if (state == kStreamStatePaused) {
        // state_, not state, by intention.
        logger_->error("Paused, taking a nap");
        condvar_.wait(lock, [this] { return state_ != kStreamStatePaused; });
        logger_->error("rising from under, resuming");
      }
    }
    cv::Mat frame = GenerateFrame();
    source_pad_->PushFrame(frame);
  } while (state != kStreamStateStopped);
  CleanupSource();
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
 *   └───────────┘   stop()     └────────────┘
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
  }
  return true;
}

bool BaseSource::Step() {
  if (state_ != kStreamStatePaused) {
    logger_->warn("Source is not paused.");
    return false;
  }
  condvar_.notify_one();
  return true;
}

StreamState BaseSource::GetState() { return state_; }

// Mat BaseSource::GenerateFrame() {}
// void BaseSource::InitializeSource() {}
// void BaseSource::CleanupSource() {}