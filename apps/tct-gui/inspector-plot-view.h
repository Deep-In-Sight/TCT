#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <implot.h>
#include <sdk/inspector/inspector-histogram.h>
#include <sdk/inspector/inspector-scanner.h>
#include <sdk/inspector/inspector-tracker.h>

#include <mutex>

#include "graphics-item-impl.h"
#include "imgui-widget.h"
#include "utility.h"

struct PlotConfigWidget;

struct PlotWidget : public ImGuiWidget {
  PlotWidget(const std::string& name);

  virtual void ImGuiDraw() override;

  bool isOpened;
  int windowFlags;
  static const char* plotHelpText;
  std::shared_ptr<InspectorMarker> markerItem;
  std::shared_ptr<PlotConfigWidget> configWidget;
  std::mutex renderMutex_;
};

struct PlotConfigWidget : public ImGuiWidget {
  virtual void ImGuiDraw() override;
  std::shared_ptr<PlotWidget> plotWidget;
  void editAPoint(ImVec2i& p, const ImRecti& bb, const std::string& name,
                  bool horiz = false);
  void editARect(ImRecti& r, const ImRecti& bb, const std::string& name);
};

struct LineScannerPlotWidget : public PlotWidget, public InspectorScanner {
  LineScannerPlotWidget(const std::string& name);
  void ImGuiDraw() override;
  void RenderRange(const std::vector<float>& vec) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;
};

struct LineScannerPlotConfigWidget : public PlotConfigWidget {
  void ImGuiDraw() override;
  ImRecti line;
};

struct HistogramPlotWidget : public PlotWidget, public InspectorHistogram {
  HistogramPlotWidget(const std::string& name);
  void ImGuiDraw() override;
  void RenderHistogram(const Mat& image) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;
  bool firstRun = true;
};

struct HistogramPlotConfigWidget : public PlotConfigWidget {
  HistogramPlotConfigWidget();
  void ImGuiDraw() override;
  ImRecti rect;
  float ranges[2];
  int bins;
  bool isAutoRange;
};

struct PointTrackerPlotWidget : public PlotWidget, public InspectorTracker {
  PointTrackerPlotWidget(const std::string& name);
  void ImGuiDraw() override;
  void RenderPoint(float value) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;
  ScrollingBuffer buffer;
};

struct PointTrackerPlotConfigWidget : public PlotConfigWidget {
  void ImGuiDraw() override;
  ImVec2i point;
};
