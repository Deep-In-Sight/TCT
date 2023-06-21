#include <sdk/calib/fisheye.h>

Fisheye::Fisheye(const string &name) : BaseTransform(name) {
  shape_ = MatShape({2, 480, 640});
  type_ = CV_32FC1;
  enabled_ = true;
}

Fisheye::~Fisheye() {}

void Fisheye::SetEnable(bool enable) {
  enabled_ = enable;
  if (enable) {
    // send valid shape to downstream
    PrepareParams();
  } else {
    // send original shape to downstream
    GetSourcePad()->SetFrameFormat(shape_, type_);
  }
}

void Fisheye::SetFrameFormat(const MatShape &shape, int type) {
  if (shape.dims() != 3 || type != CV_32FC1)
    throw std::runtime_error(
        "Fisheye only support 3 dimension CV_32FC1 format");
  shape_ = shape;
  type_ = type;
  PrepareParams();
}

void Fisheye::SetParams(FisheyeParams &params) {
  params_ = params;
  PrepareParams();
}

void Fisheye::PrepareParams() {
  int height = shape_[1];
  int width = shape_[2];
  float camMat[] = {params_.fx, 0, params_.cx, 0, params_.fy,
                    params_.cy, 0, 0,          1};
  float distCoeffs[] = {params_.k1, params_.k2, params_.p1, params_.p2,
                        params_.k3};
  float upscale = params_.upscale;
  Mat cameraMatrix = Mat(3, 3, CV_32FC1, camMat);
  Mat distCoeff = Mat(1, 5, CV_32FC1, distCoeffs);
  int width_up = (int)(width * upscale);
  int height_up = (int)(height * upscale);

  Mat newCameraMatrix =
      getOptimalNewCameraMatrix(cameraMatrix, distCoeff, Size(width, height),
                                1.0f, Size(width_up, height_up), &validRoi_);
  initUndistortRectifyMap(cameraMatrix, distCoeff, Mat(), newCameraMatrix,
                          Size(width_up, height_up), CV_32FC1, tmapx_, tmapy_);

  if (enabled_)
    GetSourcePad()->SetFrameFormat({2, validRoi_.height, validRoi_.width},
                                   type_);
}

void Fisheye::TransformFrame(Mat &frame) {
  if (!enabled_) {
    GetSourcePad()->PushFrame(frame);
    return;
  }

  int height = frame.size[1];
  int width = frame.size[2];
  int elemSize = frame.elemSize();

  Mat depth = Mat(height, width, type_, frame.data);
  Mat amplitude =
      Mat(height, width, type_, frame.data + height * width * elemSize);

  remap(depth, depth, tmapx_, tmapy_, INTER_LINEAR);
  remap(amplitude, amplitude, tmapx_, tmapy_, INTER_LINEAR);

  Mat depth_crop = depth(validRoi_);
  Mat amplitude_crop = amplitude(validRoi_);

  Mat result({2, validRoi_.height, validRoi_.width}, type_);
  Mat depth_result(validRoi_.height, validRoi_.width, type_, result.data);
  Mat amplitude_result(
      validRoi_.height, validRoi_.width, type_,
      result.data + validRoi_.height * validRoi_.width * elemSize);

  depth_crop.copyTo(depth_result);
  amplitude_crop.copyTo(amplitude_result);

  GetSourcePad()->PushFrame(result);
}
