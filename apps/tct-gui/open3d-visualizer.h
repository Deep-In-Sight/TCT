#pragma once
#include <sdk/core/pad.h>

#include <memory>
#include <thread>

namespace open3d {
namespace visualization {
namespace visualizer {
class O3DVisualizer;
}  // namespace visualizer
namespace gui {
class Window;
}  // namespace gui
namespace rendering {
class MaterialRecord;
}
}  // namespace visualization
namespace core {
class Tensor;
}  // namespace core

namespace t {
namespace geometry {
class PointCloud;
}  // namespace geometry
}  // namespace t
}  // namespace open3d

struct CameraIntrinsics {
  CameraIntrinsics();
  CameraIntrinsics(float fx, float fy, float cx, float cy, float dx, float dy);
  /**
   * @brief put the intrinsics into a 3x3 tensor
   * [[fx, 0, cx],
   *  [0, fy, cy],
   *  [0, 0, 1]]
   *
   * @return open3d::core::Tensor
   */
  open3d::core::Tensor ToTensor() const;
  float fx;  // mm
  float fy;  // mm
  float cx;  // pixel
  float cy;  // pixel
  float dx;  // micrometer
  float dy;  // micrometer
};

struct Open3DVisualizer : public PadObserver {
  Open3DVisualizer(const std::string& name);
  ~Open3DVisualizer();

  void OnNewFrame(cv::Mat& frame) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;
  void SetIntrinsics(const CameraIntrinsics& intrinsics);
  void GetIntrinsics(CameraIntrinsics& intrinsics);

  CameraIntrinsics intrinsics_;

  static std::thread* o3dApplicationThread;
  static bool appInitialized;
  static bool firstWindow;

  bool firstFrame_;
  std::mutex renderMutex_;

  static std::shared_ptr<open3d::visualization::gui::Window> currentWindow;
  std::shared_ptr<open3d::visualization::visualizer::O3DVisualizer> o3dVis_;
  std::shared_ptr<open3d::t::geometry::PointCloud> pcd;
};
