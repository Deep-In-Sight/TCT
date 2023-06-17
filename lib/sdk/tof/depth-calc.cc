#include <sdk/core/pad.h>
#include <sdk/tof/depth-calc.h>

DepthCalc::DepthCalc(const string &name) : BaseTransform(name) {}

DepthCalc::~DepthCalc() {}

void DepthCalc::SetConfig(float fmod, float offset) {
  fmod_ = fmod;
  offset_ = offset;
}

void DepthCalc::TransformFrame(Mat &frame) {
  int height = frame.size[1];
  int width = frame.size[2];
  Mat m({2, height, width}, CV_32FC1);

  int16_t *p0 = (int16_t *)frame.data;
  int16_t *p2 = (int16_t *)frame.data + height * width;
  int16_t *p1 = (int16_t *)frame.data + height * width * 2;
  int16_t *p3 = (int16_t *)frame.data + height * width * 3;

  float *depth = (float *)m.data;
  float *amplitude = (float *)m.data + height * width;

  float range = 3e8 / (2 * fmod_);
  float phase2depth_scale = 3e8 / (4 * M_PI * fmod_);

  for (int p = 0; p < height * width; p++) {
    int q = p3[p] - p1[p];
    int i = p2[p] - p0[p];
    float d = (atan2f(q, i) + M_PI) * phase2depth_scale;
    d = d + offset_;
    if (d > range) {
      d = d - range * floor(d / range);
    }
    float a = 0.5 * sqrtf(q * q + i * i);
    depth[p] = d;
    amplitude[p] = a;
  }

  GetSourcePad()->PushFrame(m);
}

void DepthCalc::SetFrameFormat(const MatShape &shape, int type) {
  const int dim = shape.dims();
  if (shape.dims() != 3 && shape[0] != 4) {
    std::invalid_argument("DepthCalc only supports 4 phase raw data");
  }
  if (type != CV_16SC1) {
    std::invalid_argument("DepthCalc only supports CV_16SC1 raw data type");
  }

  GetSourcePad()->SetFrameFormat({2, shape[1], shape[2]}, CV_32FC1);
}