#pragma once

#include <sdk/inspector/inspector-histogram.h>
#include <sdk/inspector/inspector-scanner.h>
#include <sdk/inspector/inspector-tracker.h>

#include "imgui-widget.h"

struct HLineScannerView : public ImGuiWidget, public InspectorHScanner {
  HLineScannerView(const std::string& name);
  void ImGuiDraw() override;
  void ImGuiLayout() override;
  void RenderRange(const std::vector<float>& vec) override;
};

struct VLineScannerView : public ImGuiWidget, public InspectorVScanner {
  VLineScannerView(const std::string& name);
  void ImGuiDraw() override;
  void ImGuiLayout() override;
  void RenderRange(const std::vector<float>& vec) override;
};

struct HistogramView : public ImGuiWidget, public InspectorHistogram {
  HistogramView(const std::string& name);
  void ImGuiDraw() override;
  void ImGuiLayout() override;
  void RenderHistogram(const Mat& histogram) override;
};

struct TrackerView : public ImGuiWidget, public InspectorTracker {
  TrackerView(const std::string& name);
  void ImGuiDraw() override;
  void ImGuiLayout() override;
  void RenderPoint(float value) override;
};