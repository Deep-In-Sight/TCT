#include "inspector-tools.h"

#include <implot.h>

#include <opencv2/core/core.hpp>

#include "graphics-item-impl.h"
#include "utility.h"

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

LineScannerView::LineScannerView(const std::string &name)
    : PlotViewWindow(name) {}

void LineScannerView::ImGuiDraw() {
  if (!isOpened) {
    return;
  }

  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    int xmin = start_x_;
    int ymin = start_y_;
    int xmax = end_x_;
    int ymax = end_y_;

    ImGui::PushItemWidth(50);

    ImGui::BeginGroup();
    ImGui::DragInt("Line.Min.x", &xmin, 1.0f, 0, 640);
    ImGui::DragInt("Line.Min.y", &ymin, 1.0f, 0, 480);
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::DragInt("Line.Max.x", &xmax, 1.0f, 0, 640);
    ImGui::DragInt("Line.Max.y", &ymax, 1.0f, 0, 480);
    ImGui::EndGroup();

    ImGui::PopItemWidth();

    SetRoi(xmin, ymin, xmax, ymax);

    auto line = dynamic_pointer_cast<LineMarker>(graphicsItem);
    line->modify(ImVec2(xmin, ymin), ImVec2(xmax, ymax));

    if (ImPlot::BeginPlot(name_.c_str())) {
      auto ylabel = (channel_ == kDepthChannel) ? "depth" : "intensity";
      ImPlot::SetupAxes("samples", ylabel, ImPlotAxisFlags_AutoFit,
                        ImPlotAxisFlags_AutoFit);
      ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
      ImPlot::PlotLine("LineScanner", &collected_[0], collected_.size());
      ImPlot::EndPlot();
    }

    graphicsItem->update();
  }

  ImGui::End();
}

void LineScannerView::RenderRange(const std::vector<float> &vec) {}

void LineScannerView::OnFrameFormatChanged(const MatShape &shape, int type) {}

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
    ImGui::PushItemWidth(50);
    ImGui::BeginGroup();
    ImGui::DragInt("Roi.Min.x", &x0, 1.0f, 0, 640);
    ImGui::DragInt("Roi.Min.y", &y0, 1.0f, 0, 480);
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::DragInt("Roi.Max.x", &x1, 1.0f, 0, 640);
    ImGui::DragInt("Roi.Max.y", &y1, 1.0f, 0, 480);
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Checkbox("AutoRange", &isAutoRange_);
    ImGui::DragInt("Bins", &numBins, 1.0f, 0, 500);
    ImGui::EndGroup();
    if (!isAutoRange_) {
      ImGui::SameLine();
      ImGui::BeginGroup();
      ImGui::DragFloat("Min", &rangeMin, 0.1f, 0, 100);
      ImGui::DragFloat("Max", &rangeMax, 0.1f, 0, 100);
      ImGui::EndGroup();
      SetRanges(rangeMin, rangeMax);
    }
    ImGui::PopItemWidth();

    SetBins(numBins);
    SetRoi(x0, y0, x1, y1);
    auto rect = dynamic_pointer_cast<RectMarker>(graphicsItem);
    rect->modify(ImVec2(x0, y0), ImVec2(x1, y1));

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
    SetBins(200);
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

    ImGui::PushItemWidth(50);

    ImGui::DragInt("x", &x, 1.0f, 0, 640);
    ImGui::SameLine();
    ImGui::DragInt("y", &y, 1.0f, 0, 480);

    ImGui::PopItemWidth();

    SetLocation(x, y);
    auto crosshair = dynamic_pointer_cast<CrossHairMarker>(graphicsItem);
    crosshair->modify(ImVec2(x, y));

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