#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <implot.h>
#include <sdk/inspector/inspector-bitmap.h>

#include <memory>
#include <mutex>
#include <queue>

#include "graphics-view.h"
#include "imgui-widget.h"

struct InspectorGraphicsView;
struct ColormapConfig;
struct Inspector2D : public ImGuiWidget, public InspectorBitmap {
  Inspector2D();
  ~Inspector2D();

  void ImGuiDraw() override;
  void ImGuiLayout() override;

  void DrawMenu();
  void ShowColormapSettingsPopup();
  void ShowToolsSettingsPopup();
  void HandleMouse();

  void Render(cv::Mat& frame) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;

  void AddLineScanner(const std::string& name, int channel, ImRect line);
  void AddHistogram(const std::string& name, int channel, ImRect rect);
  void AddTracker(const std::string& name, int channel, ImVec2 point);

  std::shared_ptr<GraphicsScene> scene_;
  std::shared_ptr<InspectorGraphicsView> view_;

  bool firstFrame;
  bool imageSizeChanged;
  bool windowChanged;
  bool markersDecorated;
  std::vector<cv::Mat> currentImages_;

  std::vector<ColormapConfig> cmapConfigs_;

  ImPlotContext* plotContext_;

  bool frameRendered_;
  std::mutex renderMutex_;

  ImVec2 lastWindowPos = ImVec2(0.0f, 0.0f);
  ImVec2 lastWindowSize = ImVec2(0.0f, 0.0f);
  ImVec2 lastMousePos = ImVec2(0.0f, 0.0f);
};