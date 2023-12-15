#include "open3d-visualizer.h"

// #include <open3d/t/geometry/PointCloud.h>
// #include <open3d/visualization/gui/Application.h>
// #include <open3d/visualization/rendering/Scene.h>
// #include <open3d/visualization/visualizer/O3DVisualizer.h>
#include <imgui.h>
#include <open3d/Open3D.h>

using namespace open3d;
using namespace open3d::core;
using namespace open3d::visualization::rendering;

std::thread* Open3DVisualizer::o3dApplicationThread = nullptr;
bool Open3DVisualizer::appInitialized = false;
std::shared_ptr<open3d::visualization::gui::Window>
    Open3DVisualizer::currentWindow = nullptr;
bool Open3DVisualizer::firstWindow = true;

Open3DVisualizer::Open3DVisualizer(const std::string& name)
    : PadObserver(name) {
  firstFrame_ = true;
  std::cout << "current context in main thread:" << ImGui::GetCurrentContext()
            << std::endl;
  if (o3dApplicationThread == nullptr) {
    o3dApplicationThread = new std::thread([]() {
      auto& app = open3d::visualization::gui::Application::GetInstance();
      app.Initialize();
      appInitialized = true;
      currentWindow = std::make_shared<open3d::visualization::gui::Window>(
          "Open3D", 1280, 720);
      app.AddWindow(currentWindow);
      app.Run();
    });
  }
  while (!appInitialized) {
  }
  auto& app = open3d::visualization::gui::Application::GetInstance();
  app.PostToMainThread(currentWindow.get(), [this, name]() {
    auto& app = open3d::visualization::gui::Application::GetInstance();
    o3dVis_ = std::make_shared<visualization::visualizer::O3DVisualizer>(
        name, 1280, 720);
    app.AddWindow(o3dVis_);
    if (firstWindow) {
      firstWindow = false;
      app.RemoveWindow(currentWindow.get());
    }
    currentWindow = o3dVis_;
  });
}

Open3DVisualizer::~Open3DVisualizer() {}

void Open3DVisualizer::OnNewFrame(cv::Mat& frame) {
  int h = frame.size[0];
  int w = frame.size[1];

  {
    std::lock_guard<std::mutex> lock(renderMutex_);
    m_ = frame;
  }
  auto& app = open3d::visualization::gui::Application::GetInstance();
  app.PostToMainThread(o3dVis_.get(), [this, h, w]() {
    std::lock_guard<std::mutex> lock(renderMutex_);
    core::Tensor points(m_.ptr<float>(), {h * w, 3}, Float32, Device("CPU:0"));
    auto pcd = std::make_shared<t::geometry::PointCloud>(points);
    if (firstFrame_) {
      o3dVis_->AddGeometry("cloud", pcd);
      o3dVis_->ShowGeometry("cloud", true);
      o3dVis_->ResetCameraToDefault();
      firstFrame_ = false;
    } else {
      o3dVis_->UpdateGeometry("cloud", pcd, Scene::kUpdatePointsFlag);
    }
  });
}

void Open3DVisualizer::OnFrameFormatChanged(const MatShape& shape, int type) {}