#include "inspector-tools.h"

#include <implot.h>

#include <opencv2/core/core.hpp>

#include "graphics-item-impl.h"

PlotViewWindow::PlotViewWindow(const std::string &name)
    : ImGuiWidget(name), isOpened(true) {
  windowFlags =
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
}

void PlotViewWindow::ImGuiDraw() {
  if (!isOpened) {
    return;
  }

  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    ImGui::Text("PlotViewWindow");
  }

  ImGui::End();
}

void PlotViewWindow::ImGuiLayout() {}

HLineScannerView::HLineScannerView(const std::string &name)
    : PlotViewWindow(name) {}

void HLineScannerView::ImGuiDraw() {
  if (!isOpened) {
    return;
  }

  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    ImGui::Text("HLineScannerView");
  }
  ImGui::End();
}

void HLineScannerView::RenderRange(const std::vector<float> &vec) {}

void HLineScannerView::OnFrameFormatChanged(const MatShape &shape, int type) {}

VLineScannerView::VLineScannerView(const std::string &name)
    : PlotViewWindow(name) {}

void VLineScannerView::ImGuiDraw() {
  if (!isOpened) {
    return;
  }
  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    ImGui::Text("VLineScannerView");
  }
  ImGui::End();
}

void VLineScannerView::RenderRange(const std::vector<float> &vec) {}

void VLineScannerView::OnFrameFormatChanged(const MatShape &shape, int type) {}

HistogramView::HistogramView(const std::string &name) : PlotViewWindow(name) {}

void HistogramView::ImGuiDraw() {
  if (!isOpened) {
    return;
  }
  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    int x0 = roi_.x;
    int y0 = roi_.y;
    int x1 = roi_.x + roi_.width - 1;
    int y1 = roi_.y + roi_.height - 1;
    float rangeMin = ranges_[0];
    float rangeMax = ranges_[1];
    int numBins = bins_;
    ImGui::SetNextItemWidth(120);
    ImGui::DragInt("x0", &x0, 1.0f, 0, 640);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::DragInt("y0", &y0, 1.0f, 0, 480);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::DragInt("x1", &x1, 1.0f, 0, 640);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::DragInt("y1", &y1, 1.0f, 0, 480);
    SetRoi(x0, y0, x1, y1);

    ImGui::SetNextItemWidth(120);
    ImGui::Checkbox("AutoRange", &isAutoRange_);
    if (!isAutoRange_) {
      ImGui::SameLine();
      ImGui::SetNextItemWidth(120);
      ImGui::DragFloat("Min", &rangeMin, 0.1f, 0, 100);
      ImGui::SameLine();
      ImGui::SetNextItemWidth(120);
      ImGui::DragFloat("Max", &rangeMax, 0.1f, 0, 100);
      SetRanges(rangeMin, rangeMax);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::DragInt("Bins", &numBins, 1.0f, 0, 500);
    SetBins(numBins);

    auto rectItem = dynamic_pointer_cast<GraphicRectItem>(graphicsItem);
    if (rectItem) {
      rectItem->modify(ImRect(x0, y0, x1, y1));
    }

    static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

    if (ImPlot::BeginPlot(name_.c_str())) {
      ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit,
                        ImPlotAxisFlags_AutoFit);
      ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
      ImPlot::PlotHistogram(
          "Normalized Density", (float *)histogram_.data, histogram_.total(),
          numBins, 1.0,
          (!isAutoRange_) ? ImPlotRange(rangeMin, rangeMax) : ImPlotRange(),
          ImPlotHistogramFlags_Density);
      ImPlot::EndPlot();
    }
  }
  ImGui::End();
}

void HistogramView::RenderHistogram(const Mat &histogram) {
  if (firstRun) {
    double min, max;
    cv::minMaxLoc(histogram, &min, &max);
    float minRange = (min + max) / 2 - (max - min) / 2 * 1.2;
    float maxRange = (min + max) / 2 + (max - min) / 2 * 1.2;
    SetRanges(minRange, maxRange);
    SetBins(50);
    firstRun = false;
  }
}

void HistogramView::OnFrameFormatChanged(const MatShape &shape, int type) {}

TrackerView::TrackerView(const std::string &name)
    : PlotViewWindow(name), buffer(300) {}

void TrackerView::ImGuiDraw() {
  if (!isOpened) {
    return;
  }

  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    int x = point_x;
    int y = point_y;
    ImGui::DragInt("x", &x, 1.0f, 0, 640);
    ImGui::SameLine();
    ImGui::DragInt("y", &y, 1.0f, 0, 480);
    SetLocation(x, y);
    graphicsItem->setPos(ImVec2(x, y));

    static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

    if (ImPlot::BeginPlot(name_.c_str(), ImVec2(-1, 250))) {
      ImPlot::SetupAxes("sample", nullptr);
      ImPlot::SetupAxisLimits(ImAxis_X1, buffer.Total - buffer.MaxSize,
                              buffer.Total, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 10);
      {
        std::lock_guard<std::mutex> lock(renderMutex_);
        if (!buffer.IsEmpty()) {
          ImPlot::PlotLine(name_.c_str(), &buffer.Data[0].x, &buffer.Data[0].y,
                           buffer.Data.size(), 0, buffer.Offset,
                           2 * sizeof(float));
        }
      }
      ImPlot::EndPlot();
    }
  }
  ImGui::End();
}

void TrackerView::RenderPoint(float value) {
  std::lock_guard<std::mutex> lock(renderMutex_);
  auto t = buffer.Total;
  buffer.AddPoint(t++, value);
}

void TrackerView::OnFrameFormatChanged(const MatShape &shape, int type) {}