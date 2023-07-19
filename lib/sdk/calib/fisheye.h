#ifndef __FISHEYE_H__
#define __FISHEYE_H__

#include <sdk/core/base-transform.h>

#include <opencv2/opencv.hpp>

struct FisheyeParams {
  float fx;
  float fy;
  float cx;
  float cy;
  float k1;
  float k2;
  float k3;
  float p1;
  float p2;
  float upscale;
};

class Fisheye : public BaseTransform {
 public:
  Fisheye(const string &name = "");
  ~Fisheye();

  void SetParams(FisheyeParams &params);
  void SetEnable(bool enable);

  void TransformFrame(Mat &frame) override;
  void SetFrameFormat(const MatShape &shape, int type) override;

 private:
  void PrepareParams();

 private:
  Mat tmapx_;
  Mat tmapy_;
  cv::Rect validRoi_;
  MatShape shape_;
  FisheyeParams params_;
  int type_;
  bool enabled_;
};

#endif  // __FISHEYE_H__