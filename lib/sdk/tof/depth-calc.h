#ifndef __DEPTH_CALC_H__
#define __DEPTH_CALC_H__

#include <sdk/core/base-transform.h>

class DepthCalc : public BaseTransform {
 public:
  DepthCalc(const string &name = "");
  ~DepthCalc();

  void SetConfig(float fmod, float offset);

 private:
  void TransformFrame(Mat &frame) override;
  void SetFrameFormat(const MatShape &shape, int type) override;

  float fmod_;
  float offset_;
};

#endif  //__DEPTH_CALC_H__