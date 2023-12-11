#include <sdk/tof/unprojection.h>

PinholeParams::PinholeParams() {
  fx_ = default_fx;
  fy_ = default_fy;
  cx_ = default_cx;
  cy_ = default_cy;
  dx_ = default_dx;
  dy_ = default_dy;
}

PinholeParams::PinholeParams(float fx, float fy, float cx, float cy, float dx,
                             float dy) {
  fx_ = fx;
  fy_ = fy;
  cx_ = cx;
  cy_ = cy;
  dx_ = dx;
  dy_ = dy;
}

PinholeParams PinholeParams::DefaultParams() {
  return PinholeParams(default_fx, default_fy, default_cx, default_cy,
                       default_dx, default_dy);
}

const float PinholeParams::default_fx = 7.3e-3;
const float PinholeParams::default_fy = 7.3e-3;
const float PinholeParams::default_cx = 320;
const float PinholeParams::default_cy = 240;
const float PinholeParams::default_dx = 10e-6;
const float PinholeParams::default_dy = 10e-6;

Unprojection::Unprojection(const string& name) : BaseTransform(name) {
  params_ = PinholeParams::DefaultParams();
}

Unprojection::~Unprojection() {}

void Unprojection::SetParams(PinholeParams& params) { params_ = params; }

PinholeParams Unprojection::GetParams() { return params_; }

void Unprojection::TransformFrame(Mat& frame) {
  int height, width;
  MatShape shape(frame.size);
  height = (shape.dims() == 3) ? shape[1] : shape[0];
  width = (shape.dims() == 3) ? shape[2] : shape[1];
  Mat cloud({height, width, 3}, CV_32FC1);

  float* cloudPtr = (float*)cloud.data;
  float* z = (float*)frame.data;
  // TODO: cache the xyz coefficients into another Mat to avoid repetitive
  // calculation also cache the cloud Mat to avoid repetitive allocation
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++, cloudPtr += 3, z++) {
      cloudPtr[0] = (*z) * (x - params_.cx_) * params_.dx_ / params_.fx_;
      cloudPtr[1] = (*z) * (y - params_.cy_) * params_.dy_ / params_.fy_;
      cloudPtr[2] = (*z);
    }
  }

  GetSourcePad()->PushFrame(cloud);
}

void Unprojection::SetFrameFormat(const MatShape& shape, int type) {
  int height, width;
  height = (shape.dims() == 3) ? shape[1] : shape[0];
  width = (shape.dims() == 3) ? shape[2] : shape[1];

  MatShape cloudShape(height, width, 3);  // xyz
  int cloudType = CV_32FC1;

  GetSourcePad()->SetFrameFormat(cloudShape, cloudType);
}