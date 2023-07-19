#include <sdk/core/queue.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;
static logger* logger_ = stdout_color_mt("Queue").get();

Queue::Queue(const string& name) : Element(name) {
  stop_thread_ = false;
  src_ = new Pad(kPadSource, "src");
  sink_ = new Pad(kPadSink, "sink");
  AddPad(src_);
  AddPad(sink_);
  max_queue_depth_ = DEFAULT_QUEUE_DEPTH;
  thread_ = new thread(&Queue::WaitFrame, this);
}

Queue::~Queue() {
  stop_thread_ = true;
  condvar_.notify_one();
  thread_->join();
  while (!queue_.empty()) {
    queue_.pop();
  }
  delete thread_;
  delete src_;
  delete sink_;
}

void Queue::SetMaxQueueDepth(int max_queue_depth) {
  max_queue_depth_ = max_queue_depth;
}

int Queue::GetQueueDepth() { return queue_.size(); }

Pad* Queue::GetSourcePad() { return src_; }

Pad* Queue::GetSinkPad() { return sink_; }

void Queue::PushFrame(cv::Mat& frame) {
  {
    lock_guard<mutex> lock(mutex_);
    if (queue_.size() == max_queue_depth_) {
      logger_->warn("Queue is full, drop first frame.");
      queue_.pop();
    }
    queue_.push(frame);
  }
  condvar_.notify_one();
}

void Queue::WaitFrame() {
  cv::Mat frame;
  while (!stop_thread_) {
    {
      unique_lock<mutex> lock(mutex_);
      condvar_.wait(lock, [this] { return !queue_.empty() || stop_thread_; });
      if (stop_thread_) {
        break;
      }
      frame = queue_.front();
      queue_.pop();
    }

    src_->PushFrame(frame);
  }
}

void Queue::PushState(StreamState state) { src_->PushState(state); }