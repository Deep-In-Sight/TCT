#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <implot.h>
#include <sdk/inspector/inspector-histogram.h>
#include <sdk/inspector/inspector-scanner.h>
#include <sdk/inspector/inspector-tracker.h>

#include <mutex>

#include "graphics-item.h"
#include "imgui-widget.h"
#include "utility.h"

struct PlotViewWindow : public ImGuiWidget {
  PlotViewWindow(const std::string& name);

  virtual void ImGuiDraw() override;

  bool isOpened;
  int windowFlags;
  std::shared_ptr<GraphicsItem> graphicsItem;
  std::mutex renderMutex_;
};

struct LineScannerView : public PlotViewWindow, public InspectorScanner {
  LineScannerView(const std::string& name);
  void ImGuiDraw() override;
  void RenderRange(const std::vector<float>& vec) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;
};

struct HistogramView : public PlotViewWindow, public InspectorHistogram {
  HistogramView(const std::string& name);
  void ImGuiDraw() override;
  void RenderHistogram(const Mat& image) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;
  bool firstRun = true;
};

struct TrackerView : public PlotViewWindow, public InspectorTracker {
  TrackerView(const std::string& name);
  void ImGuiDraw() override;
  void RenderPoint(float value) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;
  ScrollingBuffer buffer;
};
