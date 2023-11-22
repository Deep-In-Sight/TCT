#include <sdk/core/pad.h>
#include <sdk/tof/moving-average.h>
#include <spdlog/sinks/stdout_color_sinks.h>
using namespace spdlog;

static logger* logger_ = stdout_color_mt("MovingAverage").get();

MovingAverage::MovingAverage(const string& name) : BaseTransform(name) {
  windowSize_ = 4;
  listCount_ = 0;
  logger_->set_level(level::warn);
}

MovingAverage::~MovingAverage() {}

void MovingAverage::SetWindowSize(int windowSize) {
  lock_guard<mutex> lock(mutex_);
  frameList_.clear();
  windowSize_ = windowSize;
}

void MovingAverage::TransformFrame(Mat& frame) {
  lock_guard<mutex> lock(mutex_);
  if (windowSize_ <= 1) {
    GetSourcePad()->PushFrame(frame);
    return;
  }

  logger_->info("Getting new frame, data = {}", (void*)frame.data);

  if (frameList_.size() == 0) {
    frameSum_ = Mat({2, frame.size[1], frame.size[2]}, CV_32FC1, 0.0f);
  }
  frameSum_ = frameSum_ + frame;
  frameList_.push_back(frame);

  if (frameList_.size() >= windowSize_) {
    Mat frameAvg = frameSum_ / windowSize_;
    GetSourcePad()->PushFrame(frameAvg);

    frameSum_ = frameSum_ - frameList_.front();
    logger_->info("frameList_.size() = {}, removing {}", frameList_.size(),
                  (void*)frameList_.front().data);
    frameList_.pop_front();
  }
}

void MovingAverage::PushState(StreamState state) {
  lock_guard<mutex> lock(mutex_);
  if (state == kStreamStatePaused || state == kStreamStateStopped) {
    frameList_.clear();
  }
  BaseTransform::PushState(state);
}
