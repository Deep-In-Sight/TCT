#include "inspector-plot-view.h"

#include <implot.h>

#include <opencv2/core/core.hpp>

#include "graphics-item-impl.h"
#include "utility.h"

void PlotConfigWidget::ImGuiDraw() {}

void PlotConfigWidget::editAPoint(ImVec2i &p, const ImRecti &bb,
                                  const std::string &name, bool horiz) {
  auto xlabel = (name + ".X");
  auto ylabel = (name + ".Y");
  ImGui::PushItemWidth(50);
  ImGui::BeginGroup();
  ImGui::DragInt(xlabel.c_str(), (int *)&p.x, 1.0f, bb.Min.x, bb.Max.x);
  if (horiz) ImGui::SameLine();
  ImGui::DragInt(ylabel.c_str(), (int *)&p.y, 1.0f, bb.Min.y, bb.Max.y);
  ImGui::EndGroup();
  ImGui::PopItemWidth();
};
void PlotConfigWidget::editARect(ImRecti &r, const ImRecti &bb,
                                 const std::string &name) {
  ImGui::Checkbox("Full Range", &isFullRange);
  if (isFullRange) {
    r.Min = bb.Min;
    r.Max = bb.Max;
  }
  editAPoint(r.Min, bb, name + ".Min");
  ImGui::SameLine();
  editAPoint(r.Max, bb, name + ".Max");
  if (r.Min.x > r.Max.x) std::swap(r.Min.x, r.Max.x);
  if (r.Min.y > r.Max.y) std::swap(r.Min.y, r.Max.y);
};

PlotWidget::PlotWidget(const std::string &name)
    : ImGuiWidget(name), isOpened(true) {
  windowFlags = ImGuiWindowFlags_NoSavedSettings;
}

void PlotWidget::ImGuiDraw() {
  if (!isOpened) {
    return;
  }

  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    ImGui::Text("PlotViewWindow");
  }

  ImGui::End();
}

const char *PlotWidget::plotHelpText =
    "Right click on legend and plot area to open context menu.\n"
    "Right click and drag to zoom area.\n"
    "Double click to fit all data.";

LineScannerPlotWidget::LineScannerPlotWidget(const std::string &name)
    : PlotWidget(name) {}

void LineScannerPlotWidget::ImGuiDraw() {
  if (!isOpened) {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_Once);
  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    configWidget->ImGuiDraw();
    ImGui::SameLine();
    HelpMarker(plotHelpText);

    if (ImPlot::BeginPlot("##NoTitle", ImVec2(-1, -1))) {
      auto ylabel = (channel_ == kDepthChannel) ? "depth" : "intensity";
      ImPlot::SetupAxes("samples", ylabel, ImPlotAxisFlags_AutoFit,
                        ImPlotAxisFlags_AutoFit);
      ImPlot::PlotLine("LineScanner", &collected_[0], collected_.size());
      ImPlot::EndPlot();
    }

    markerItem->update();
  }

  ImGui::End();
}

void LineScannerPlotWidget::RenderRange(const std::vector<float> &vec) {}

void LineScannerPlotWidget::OnFrameFormatChanged(const MatShape &shape,
                                                 int type) {}

void LineScannerPlotConfigWidget::ImGuiDraw() {
  auto lineScanner = dynamic_pointer_cast<LineScannerPlotWidget>(plotWidget);
  ImRecti bb(0, 0, 639, 479);

  if (lineScanner) {
    MatShape shape;
    int type;
    lineScanner->GetRoi(line.Min.x, line.Min.y, line.Max.x, line.Max.y);
    lineScanner->GetPad()->GetFrameFormat(shape, type);
    bb = ImRecti(0, 0, shape[2] - 1, shape[1] - 1);
  }

  editARect(line, bb, "Line");

  if (lineScanner) {
    lineScanner->SetRoi(line.Min.x, line.Min.y, line.Max.x, line.Max.y);

    auto lineMarker = dynamic_pointer_cast<LineMarker>(lineScanner->markerItem);
    lineMarker->modify(line.Min, line.Max);
  }
}

HistogramPlotWidget::HistogramPlotWidget(const std::string &name)
    : PlotWidget(name) {}

void HistogramPlotWidget::ImGuiDraw() {
  if (!isOpened) {
    return;
  }
  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Once);
  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    configWidget->ImGuiDraw();
    ImGui::SameLine();
    HelpMarker(plotHelpText);

    // TODO: move the histogram calculation down to the sdk
    auto hc = dynamic_pointer_cast<HistogramPlotConfigWidget>(configWidget);
    int numBins = hc->bins;
    float rangeMin = hc->ranges[0];
    float rangeMax = hc->ranges[1];
    bool isAutoRange = hc->isAutoRange;

    if (ImPlot::BeginPlot(name_.c_str(), ImVec2(-1, -1))) {
      ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit,
                        ImPlotAxisFlags_AutoFit);
      ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
      ImPlot::PlotHistogram(
          "Normalized Density", (float *)histogram_.data, histogram_.total(),
          numBins, 1.0,
          (!isAutoRange) ? ImPlotRange(rangeMin, rangeMax) : ImPlotRange(),
          ImPlotHistogramFlags_Density);
      ImPlot::EndPlot();
    }
  }
  ImGui::End();
}

void HistogramPlotWidget::RenderHistogram(const Mat &histogram) {}

void HistogramPlotWidget::OnFrameFormatChanged(const MatShape &shape,
                                               int type) {}

HistogramPlotConfigWidget::HistogramPlotConfigWidget() {
  isAutoRange = true;
  ranges[0] = 0;
  ranges[1] = 0;
  bins = 200;
}

void HistogramPlotConfigWidget::ImGuiDraw() {
  auto histogram = dynamic_pointer_cast<HistogramPlotWidget>(plotWidget);
  ImRecti bb(0, 0, 639, 479);

  if (histogram) {
    MatShape shape;
    int type;
    histogram->GetRoi(rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y);
    histogram->GetBins(bins);
    histogram->GetRanges(ranges[0], ranges[1]);
    isAutoRange = histogram->isAutoRange();
    histogram->GetPad()->GetFrameFormat(shape, type);
    bb = ImRecti(0, 0, shape[2] - 1, shape[1] - 1);
  }

  editARect(rect, bb, "ROI");
  ImGui::PushItemWidth(100);
  ImGui::SameLine();
  ImGui::BeginGroup();
  ImGui::Checkbox("AutoRange", &isAutoRange);
  ImGui::DragInt("Bins", &bins, 1.0f, 0, 500);
  ImGui::EndGroup();
  if (!isAutoRange) {
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::DragFloat("Min", &ranges[0], 0.1f, 0, 100);
    ImGui::DragFloat("Max", &ranges[1], 0.1f, 0, 100);
    ImGui::EndGroup();
  }
  ImGui::PopItemWidth();

  if (histogram) {
    histogram->SetRanges(ranges[0], ranges[1]);
    histogram->SetBins(bins);
    histogram->SetAutoRange(isAutoRange);
    histogram->SetRoi(rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y);

    auto rectMarker = dynamic_pointer_cast<RectMarker>(histogram->markerItem);
    rectMarker->modify(rect.Min, rect.Max);
  }
}

PointTrackerPlotWidget::PointTrackerPlotWidget(const std::string &name)
    : PlotWidget(name), buffer(300) {}

void PointTrackerPlotWidget::ImGuiDraw() {
  if (!isOpened) {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_Once);
  if (ImGui::Begin(name_.c_str(), &isOpened, windowFlags)) {
    configWidget->ImGuiDraw();

    if (ImPlot::BeginPlot(name_.c_str(), ImVec2(-1, -1))) {
      ImPlot::SetupAxes("sample", nullptr);
      ImPlot::SetupAxisLimits(ImAxis_X1, buffer.Total - buffer.MaxSize,
                              buffer.Total, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 10);
      if (!buffer.IsEmpty()) {
        ImPlot::PlotLine(name_.c_str(), &buffer.Data[0].x, &buffer.Data[0].y,
                         buffer.Data.size(), 0, buffer.Offset,
                         2 * sizeof(float));
      }
      ImPlot::EndPlot();
    }
  }
  ImGui::End();
}

void PointTrackerPlotWidget::RenderPoint(float value) {
  auto t = buffer.Total;
  buffer.AddPoint(t++, value);
}

void PointTrackerPlotWidget::OnFrameFormatChanged(const MatShape &shape,
                                                  int type) {}

void PointTrackerPlotConfigWidget::ImGuiDraw() {
  auto pointTracker = dynamic_pointer_cast<PointTrackerPlotWidget>(plotWidget);
  ImRecti bb(0, 0, 639, 479);

  if (pointTracker) {
    MatShape shape;
    int type;
    pointTracker->GetLocation(point.x, point.y);
    pointTracker->GetPad()->GetFrameFormat(shape, type);
    bb = ImRecti(0, 0, shape[2] - 1, shape[1] - 1);
  }

  editAPoint(point, bb, "Point", true);

  if (pointTracker) {
    pointTracker->SetLocation(point.x, point.y);

    auto crosshair =
        dynamic_pointer_cast<CrossHairMarker>(pointTracker->markerItem);
    crosshair->modify(point);
  }
}