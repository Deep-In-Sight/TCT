#ifndef __MOVING_AVERAGE_H__
#define __MOVING_AVERAGE_H__

#include <sdk/core/base-transform.h>

class MovingAverage : public BaseTransform {
 public:
  MovingAverage(const string &name = "");
  ~MovingAverage();

  void SetWindowSize(int windowSize);

 private:
  void TransformFrame(Mat &frame) override;
  int windowSize_;
  Mat frameSum_;
  std::list<Mat> frameList_;
  int listCount_;
  mutex mutex_;
};

#endif  //__MOVING_AVERAGE_H__