#include <sdk/core/pad.h>
#include <sdk/tof/moving-average.h>

MovingAverage::MovingAverage(const string& name) : BaseTransform(name) {
  windowSize_ = 4;
  listCount_ = 0;
}

MovingAverage::~MovingAverage() {}

void MovingAverage::SetWindowSize(int windowSize) {
  lock_guard<mutex> lock(mutex_);
  if (windowSize < listCount_) {
    frameSum_ = Mat();
    listCount_ = 0;
    frameList_.clear();
  }
  windowSize_ = windowSize;
}

void MovingAverage::TransformFrame(Mat& frame) {
  lock_guard<mutex> lock(mutex_);
  if (windowSize_ <= 1) {
    GetSourcePad()->PushFrame(frame);
    return;
  }

  if (listCount_ == 0) {
    frameSum_ = frame;
  } else {
    frameSum_ = frameSum_ + frame;
  }
  frameList_.push_back(frame);
  listCount_++;

  if (listCount_ >= windowSize_) {
    frame = frameSum_ / windowSize_;
    GetSourcePad()->PushFrame(frame);

    frameSum_ = frameSum_ - frameList_.front();
    frameList_.pop_front();
    listCount_--;
  }
}
