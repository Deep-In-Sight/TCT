#pragma once
#include <sdk/core/pad.h>

#include <memory>
#include <thread>

namespace open3d {
namespace visualization {
namespace visualizer {
class O3DVisualizer;
}
namespace gui {
class Window;
}
}  // namespace visualization
}  // namespace open3d

struct Open3DVisualizer : public PadObserver {
  Open3DVisualizer(const std::string& name);
  ~Open3DVisualizer();

  void OnNewFrame(cv::Mat& frame) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;

  static std::thread* o3dApplicationThread;
  static bool appInitialized;
  static bool firstWindow;
  static std::shared_ptr<open3d::visualization::gui::Window> currentWindow;
  std::shared_ptr<open3d::visualization::visualizer::O3DVisualizer> o3dVis_;
  bool firstFrame_;
};
