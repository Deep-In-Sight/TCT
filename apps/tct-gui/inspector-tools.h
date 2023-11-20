#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <implot.h>
#include <sdk/inspector/inspector-histogram.h>
#include <sdk/inspector/inspector-scanner.h>
#include <sdk/inspector/inspector-tracker.h>

#include <mutex>

#include "graphics-item.h"
#include "imgui-widget.h"
struct ScrollingBuffer {
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  int Total;
  ScrollingBuffer(int max_size = 2000) {
    MaxSize = max_size;
    Offset = 0;
    Total = 0;
    Data.reserve(MaxSize);
  }
  void AddPoint(float x, float y) {
    if (Data.size() < MaxSize)
      Data.push_back(ImVec2(x, y));
    else {
      Data[Offset] = ImVec2(x, y);
      Offset = (Offset + 1) % MaxSize;
    }
    Total++;
  }
  void Erase() {
    if (Data.size() > 0) {
      Data.shrink(0);
      Offset = 0;
    }
  }
  bool IsEmpty() { return Data.size() == 0; }
};

// TODO need to separate a window, and a widget, to 2 different things
struct PlotViewWindow : public ImGuiWidget {
  PlotViewWindow(const std::string& name);

  virtual void ImGuiDraw() override;
  void ImGuiLayout() override;

  bool isOpened = true;
  int windowFlags = ImGuiWindowFlags_NoSavedSettings;
  std::shared_ptr<GraphicsItem> graphicsItem;
  std::mutex renderMutex_;
};

struct HLineScannerView : public PlotViewWindow, public InspectorHScanner {
  HLineScannerView(const std::string& name);
  void ImGuiDraw() override;
  void RenderRange(const std::vector<float>& vec) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;
};

struct VLineScannerView : public PlotViewWindow, public InspectorVScanner {
  VLineScannerView(const std::string& name);
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