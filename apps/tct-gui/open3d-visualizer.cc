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
    auto car = std::make_shared<geometry::TriangleMesh>();
    io::ReadTriangleMesh("./data/Mercedes.obj", *car);
    o3dVis_->AddGeometry("car", car);
    o3dVis_->ShowGeometry("car", false);

    auto dentum = std::make_shared<geometry::TriangleMesh>();
    io::ReadTriangleMesh("./data/dentum.ply", *dentum);
    o3dVis_->AddGeometry("dentum", dentum);
    o3dVis_->ShowGeometry("dentum", true);

    auto matRecord = o3dVis_->GetGeometryMaterial("dentum");
    matRecord.shader = "defaultLit";
    o3dVis_->ModifyGeometryMaterial("dentum", &matRecord);
  });

  pcd = std::make_shared<t::geometry::PointCloud>();
}

Open3DVisualizer::~Open3DVisualizer() {}

void Open3DVisualizer::OnNewFrame(cv::Mat& frame) {
  MatShape shape(frame.size);
  int channels = (shape.dims() == 3) ? shape[0] : 1;
  int height = (shape.dims() == 3) ? shape[1] : shape[0];
  int width = (shape.dims() == 3) ? shape[2] : shape[1];

  cv::Mat depth, color;
  depth = (channels == 1)
              ? frame
              : cv::Mat(height, width, CV_32FC1, frame.ptr<float>());
  cv::normalize(depth, color, 0, 255, cv::NORM_MINMAX, CV_8UC1);
  cv::applyColorMap(color, color, cv::COLORMAP_JET);
  cv::cvtColor(color, color, cv::COLOR_BGR2RGB);

  // data is copied to tensor buffer
  core::Tensor depthTensor(depth.ptr<float>(), {height, width}, Float32);
  core::Tensor colorTensor(color.ptr<uint8_t>(), {height, width, 3}, UInt8);

  t::geometry::Image depthImage(depthTensor);
  t::geometry::Image colorImage(colorTensor);
  t::geometry::RGBDImage rgbdImage(colorImage, depthImage);
  {
    std::lock_guard<std::mutex> lock(renderMutex_);
    // the geomeotries' underlying memory is shared_ptr<Blob> so it's ok for
    // them to go out of scope, won't be deallocated as long as something is
    // still referring to it
    // *pcd = t::geometry::PointCloud::CreateFromRGBDImage(
    //     rgbdImage, intrinsics_.ToTensor(),
    //     core::Tensor::Eye(4, core::Float32, core::Device("CPU:0")),
    //     /*depth_scale=*/1.0f,
    //     /*depth_max=*/10.0f);
    *pcd = t::geometry::PointCloud::CreateFromRGBDImage(
        rgbdImage, intrinsics_.ToTensor(), extrinsics_.ToTensor(),
        /*depth_scale=*/1.0f,
        /*depth_max=*/10.0f);
  }

  auto& app = open3d::visualization::gui::Application::GetInstance();
  app.PostToMainThread(o3dVis_.get(), [this]() {
    // The mutex is to prevent frame being changed while rendering
    auto renderPcd = std::make_shared<t::geometry::PointCloud>();
    {
      std::lock_guard<std::mutex> lock(renderMutex_);
      *renderPcd = (*pcd).Clone();
    }

    if (firstFrame_) {
      o3dVis_->AddGeometry("cloud", renderPcd);
      o3dVis_->ShowGeometry("cloud", true);
      o3dVis_->ResetCameraToDefault();
      auto camera = o3dVis_->GetScene()->GetCamera();
      // center to [0,0,1], put eye at [0,0,0], up direction is [0,-1,0]
      camera->LookAt({0, 0, 1}, {0, 0, 0}, {0, -1, 0});
      firstFrame_ = false;
    } else {
      o3dVis_->UpdateGeometry(
          "cloud", renderPcd,
          Scene::kUpdatePointsFlag | Scene::kUpdateColorsFlag);
    }
  });
}

void Open3DVisualizer::OnFrameFormatChanged(const MatShape& shape, int type) {}

void Open3DVisualizer::GetIntrinsics(CameraIntrinsics& intrinsics) {
  intrinsics = intrinsics_;
}
void Open3DVisualizer::SetIntrinsics(const CameraIntrinsics& intrinsics) {
  intrinsics_ = intrinsics;
}

CameraIntrinsics::CameraIntrinsics()
    : fx(7.3), fy(7.3), cx(320), cy(240), dx(10), dy(10){};

CameraIntrinsics::CameraIntrinsics(float fx, float fy, float cx, float cy,
                                   float dx, float dy)
    : fx(fx), fy(fy), cx(cx), cy(cy), dx(dx), dy(dy){};

core::Tensor CameraIntrinsics::ToTensor() const {
  std::vector<float> mat = {
      fx * 1e3f / dx, 0, cx, 0, fy * 1e3f / dy, cy, 0, 0, 1};
  return core::Tensor(mat, {3, 3}, core::Float32);
}

CameraExtrinsics::CameraExtrinsics()
    : tx(0), ty(0), tz(0), rx(0), ry(0), rz(0){};

core::Tensor CameraExtrinsics::ToTensor() const {
  using namespace std;
  float a = rx / 180.0 * M_PI;
  float b = ry / 180.0 * M_PI;
  float g = rz / 180.0 * M_PI;
  float r00 = cos(b) * cos(g);
  float r01 = sin(a) * sin(b) * cos(g) - cos(a) * sin(g);
  float r02 = cos(a) * sin(b) * cos(g) + sin(a) * sin(g);
  float r10 = cos(b) * sin(g);
  float r11 = sin(a) * sin(b) * sin(g) + cos(a) * cos(g);
  float r12 = cos(a) * sin(b) * sin(g) - sin(a) * cos(g);
  float r20 = -sin(b);
  float r21 = sin(a) * cos(b);
  float r22 = cos(a) * cos(b);
  std::vector<float> mat = {r00, r01, r02, tx, r10, r11, r12, ty,
                            r20, r21, r22, tz, 0,   0,   0,   1};
  return core::Tensor(mat, {4, 4}, core::Float32);
}