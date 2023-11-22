#include "inspector-2d.h"

#include <iostream>
#include <opencv2/imgcodecs.hpp>

#include "graphics-item-impl.h"
#include "graphics-layout.h"
#include "inspector-graphics-view.h"
#include "inspector-tools.h"
#include "utility.h"

Inspector2D::Inspector2D() {
  scene_ = std::make_shared<GraphicsScene>();
  view_ = std::make_shared<InspectorGraphicsView>(scene_);
  view_->inspector_ = this;
  cmapConfigs_.emplace_back();
  cmapConfigs_.emplace_back();

  firstFrame = true;
  imageSizeChanged = false;
  windowChanged = false;
  markersDecorated = false;

  frameRendered_ = false;

  plotContext_ = ImPlot::CreateContext();
}

Inspector2D::~Inspector2D() { ImPlot::DestroyContext(); }

void Inspector2D::ImGuiDraw() {
  static ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

  bool needRelayout;

  ImPlot::SetCurrentContext(plotContext_);

  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
  ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);

  if (ImGui::Begin("Inspector 2D", nullptr, windowFlags)) {
    {
      std::lock_guard<std::mutex> lock(renderMutex_);
      needRelayout = firstFrame || imageSizeChanged || windowChanged ||
                     view_->layoutChanged_;
      if (!frameRendered_) {
        if (!currentImages_.empty()) {
          view_->setImages(currentImages_);
        }
        frameRendered_ = true;
      }
    }

    if (needRelayout) {
      ImGuiLayout();
    }

    // draw menus
    DrawMenu();

    // child widgets draw
    view_->ImGuiDraw();

    // handle events
    HandleMouse();

    if (firstFrame) {
      firstFrame = false;
    }

    if (ImGui::GetWindowSize() != lastWindowSize ||
        ImGui::GetWindowPos() != lastWindowPos) {
      windowChanged = true;
      lastWindowPos = ImGui::GetWindowPos();
      lastWindowSize = ImGui::GetWindowSize();
    } else {
      windowChanged = false;
    }
  }
  ImGui::End();

  for (auto& child : children) {
    child->ImGuiDraw();
    auto toolView = std::dynamic_pointer_cast<PlotViewWindow>(child);
    if (toolView && !toolView->isOpened) {
      view_->scene_->removeItem(toolView->graphicsItem);
    }
  }
}

void Inspector2D::ImGuiLayout() {
  contentRect = ImRect(ImGui::GetWindowContentRegionMin(),
                       ImGui::GetWindowContentRegionMax());
  view_->contentRect = contentRect;
  view_->ImGuiLayout();
}

void Inspector2D::DrawMenu() {
  bool showColormapPopup = false;
  bool showToolsScannerPopup = false;
  bool showToolsTrackerPopup = false;
  bool showToolsHistogramPopup = false;

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("View")) {
      if (ImGui::BeginMenu("View Mode")) {
        static bool menuOriginalSelected = true;
        static bool menuFitSelected = false;
        static bool menuStretchSelected = false;
        ViewMode lastvm = view_->viewMode_;
        ViewMode vm = view_->viewMode_;
        if (ImGui::MenuItem("Original", nullptr, &menuOriginalSelected)) {
          vm = ViewMode::kViewOriginal;
        }
        if (ImGui::MenuItem("Fit", nullptr, &menuFitSelected)) {
          vm = ViewMode::kViewFit;
        }
        if (ImGui::MenuItem("Stretch", nullptr, &menuStretchSelected)) {
          vm = ViewMode::kViewStretch;
        }
        if (lastvm != vm) {
          view_->setImageFitMode(vm);
        }
        menuOriginalSelected = (vm == ViewMode::kViewOriginal);
        menuFitSelected = (vm == ViewMode::kViewFit);
        menuStretchSelected = (vm == ViewMode::kViewStretch);
        ImGui::EndMenu();  // end menu View / View Mode
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Marker Labels", nullptr, &markersDecorated)) {
        for (auto& child : children) {
          auto view = std::dynamic_pointer_cast<PlotViewWindow>(child);
          auto marker =
              std::dynamic_pointer_cast<InspectorMarker>(view->graphicsItem);
          marker->EnableLabel(markersDecorated);
        }
      }
      bool rulersEnabled = view_->rulersEnabled_;
      if (ImGui::MenuItem("Rulers", nullptr, &rulersEnabled)) {
        view_->enableRulers(rulersEnabled);
      }
      bool sideBySide = view_->sideBySide_;
      if (ImGui::MenuItem("Side By Side", nullptr, &sideBySide)) {
        view_->enableSideBySide(sideBySide);
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Colormap", nullptr, nullptr)) {
        showColormapPopup = true;
      }

      ImGui::EndMenu();  // end menu View
    }
    if (ImGui::BeginMenu("Tools")) {
      if (ImGui::MenuItem("Line Scanner")) {
        showToolsScannerPopup = true;
      }
      if (ImGui::MenuItem("Tracker")) {
        showToolsTrackerPopup = true;
      }
      if (ImGui::MenuItem("Histogram")) {
        showToolsHistogramPopup = true;
      }
      ImGui::EndMenu();  // end menu Tools
    }
    ImGui::EndMenuBar();
  }

  if (showColormapPopup) {
    ImGui::OpenPopup("Colormap Settings");
  }
  if (showToolsTrackerPopup) {
    ImGui::OpenPopup("Tracker Settings");
  }
  if (showToolsScannerPopup) {
    ImGui::OpenPopup("Scanner Settings");
  }
  if (showToolsHistogramPopup) {
    ImGui::OpenPopup("Histogram Settings");
  }
  ShowColormapSettingsPopup();
  ShowToolsSettingsPopup();
}

struct ColormapConfig {
  bool clamp = false;
  float min = 0.0f;
  float max = 10.0f;
  bool cmap = true;
  cv::ColormapTypes cmapType = cv::COLORMAP_JET;
};

void Inspector2D::ShowColormapSettingsPopup() {
  auto center = contentRect.GetCenter() + ImGui::GetWindowPos();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("Colormap Settings", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoSavedSettings)) {
    static int index = 0;
    ColormapConfig& cc = cmapConfigs_[index];

    ImGui::Combo("Channel", &index, "Depth\0Amplitude\0");
    if (index == 0) {
      // because opencv does not have COLORMAP_NONE
      int cmapCode = cc.cmapType + 1;
      ImGui::Combo("Colormap", &cmapCode,
                   "None\0Autumn\0Bone\0Jet\0Winter\0Rainbow\0"
                   "Ocean\0Summer\0Spring\0Cool\0HSV\0"
                   "Pink\0Hot\0Parula\0Magma\0Inferno\0"
                   "Plasma\0Viridis\0Cividis\0Twilight\0"
                   "Twilight Shifted\0Turbo\0");
      cc.cmapType = (cv::ColormapTypes)(cmapCode - 1);
      cc.cmap = (cmapCode > 0);
    }
    ImGui::Checkbox("Clamp", &cc.clamp);
    float rangemax = (index == 0) ? 10 : 500;  // TODO: get from frame
    ImGui::SliderFloat("Min", &cc.min, 0.0f, rangemax);
    ImGui::SliderFloat("Max", &cc.max, 0.0f, rangemax);

    if (cc.min > cc.max) std::swap(cc.min, cc.max);

    if (ImGui::Button("OK")) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void Inspector2D::ShowToolsSettingsPopup() {
  auto center = contentRect.GetCenter() + ImGui::GetWindowPos();

  auto popupFlags =
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;

  auto editAPoint = [&](ImVec2& p, const std::string& name,
                        bool disableX = false, bool disableY = false) {
    auto xlabel = (name + ".X");
    auto ylabel = (name + ".Y");
    int x = (int)p.x;
    int y = (int)p.y;
    ImGui::BeginGroup();
    if (!disableX)
      ImGui::DragInt(xlabel.c_str(), (int*)&x, 1.0f, 0,
                     view_->imageItems_[0]->imageSize_.x);
    if (!disableY)
      ImGui::DragInt(ylabel.c_str(), (int*)&y, 1.0f, 0,
                     view_->imageItems_[0]->imageSize_.y);
    ImGui::EndGroup();
    p.x = x;
    p.y = y;
  };
  auto editARect = [&](ImRect& r, const std::string& name,
                       bool disableMaxX = false, bool disableMaxY = false) {
    editAPoint(r.Min, name + ".Min");
    ImGui::SameLine();
    editAPoint(r.Max, name + ".Max", disableMaxX, disableMaxY);
    if (disableMaxX)
      r.Max.x = r.Min.x;
    else if (r.Min.x > r.Max.x)
      std::swap(r.Min.x, r.Max.x);
    if (disableMaxY)
      r.Max.y = r.Min.y;
    else if (r.Min.y > r.Max.y)
      std::swap(r.Min.y, r.Max.y);
  };

  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Tracker Settings", nullptr, popupFlags)) {
    ImGui::PushItemWidth(100);
    static ImVec2 uv;
    static int channel = 0;
    static int id = 0;
    ImGui::Combo("Channel", &channel, "Depth\0Amplitude\0");
    editAPoint(uv, "p");
    ImGui::PopItemWidth();
    if (ImGui::Button("OK")) {
      std::string name = "Tracker " + std::to_string(id++);
      AddTracker(name, channel, uv);
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Scanner Settings", nullptr, popupFlags)) {
    static ImRect line;
    static int channel = 0;
    static int id = 0;
    ImGui::PushItemWidth(100);
    ImGui::Combo("Channel", &channel, "Depth\0Amplitude\0");
    editARect(line, "Line", false, false);
    ImGui::PopItemWidth();
    if (ImGui::Button("OK")) {
      std::string name = "Scanner " + std::to_string(id++);
      AddLineScanner(name, channel, line);
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Histogram Settings", nullptr, popupFlags)) {
    static ImRect rect;
    static int channel = 0;
    static int id = 0;
    ImGui::PushItemWidth(100);
    ImGui::Combo("Channel", &channel, "Depth\0Amplitude\0");
    editARect(rect, "Roi");
    ImGui::PopItemWidth();
    if (ImGui::Button("OK")) {
      std::string name = "Histogram " + std::to_string(id++);
      AddHistogram(name, channel, rect);
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  // ImGui::PopItemWidth();
}

void Inspector2D::HandleMouse() {
  auto& io = ImGui::GetIO();

  if (ImGui::IsWindowHovered()) {
    auto mousePos = io.MousePos;
    view_->onMouseMove(mousePos);

    if (io.MouseWheel != 0.0f) {
      view_->onMouseScroll(mousePos, io.MouseWheel);
    }

    if (ImGui::IsMouseClicked(0)) {
      auto item = view_->scene_->itemAt(mousePos);
      if (item) {
        std::cout << item->name_ << " clicked" << std::endl;
      }
    }
  }
}

// implement the InspectorBitmap interface.

void Inspector2D::Render(cv::Mat& frame) {
  static bool firstImage = true;
  auto images = splitChannels(frame_);

  for (int i = 0; i < 2; i++) {
    auto& image = images[i];
    auto& config = cmapConfigs_[i];
    if (config.clamp) {
      cv::Mat minMask, maxMask;
      minMask = image < config.min;
      maxMask = image > config.max;
      image.setTo(config.min, minMask);
      image.setTo(config.max, maxMask);
    }

    cv::normalize(image, image, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    if (i == 0 && config.cmap) cv::applyColorMap(image, image, config.cmapType);
  }

  {
    std::lock_guard<std::mutex> lock(renderMutex_);
    currentImages_ = images;
    if (firstImage) {
      imageSizeChanged = true;
      firstImage = false;
    } else {
      imageSizeChanged = false;
    }
    frameRendered_ = false;
  }
}

void Inspector2D::OnFrameFormatChanged(const MatShape& shape, int type) {
  imageSizeChanged = true;
}

void Inspector2D::AddLineScanner(const std::string& name, int channel,
                                 ImRect line) {
  auto scanner = std::make_shared<LineScannerView>(name);
  this->GetPad()->AddObserver(scanner.get());
  scanner->SelectChannel((DepthAmplitudeChannel)channel);
  scanner->SetRoi(line.Min.x, line.Min.y, line.Max.x, line.Max.y);

  auto lineMarker = std::make_shared<LineMarker>(line.Min, line.Max, name);
  lineMarker->EnableLabel(markersDecorated);

  auto imageItem = view_->imageItems_[channel];
  imageItem->addChild(lineMarker);

  scanner->graphicsItem = lineMarker;
  children.push_back(scanner);
}

void Inspector2D::AddHistogram(const std::string& name, int channel,
                               ImRect rect) {
  auto histogram = std::make_shared<HistogramView>(name);
  this->GetPad()->AddObserver(histogram.get());
  histogram->SelectChannel((DepthAmplitudeChannel)channel);
  histogram->SetRoi(rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y);

  auto rectItem = std::make_shared<RectMarker>(rect.Min, rect.Max, name);
  rectItem->EnableLabel(markersDecorated);

  auto imageItem = view_->imageItems_[channel];
  imageItem->addChild(rectItem);

  histogram->graphicsItem = rectItem;
  children.push_back(histogram);
}

void Inspector2D::AddTracker(const std::string& name, int channel,
                             ImVec2 point) {
  auto tracker = std::make_shared<TrackerView>(name);
  this->GetPad()->AddObserver(tracker.get());
  tracker->SelectChannel((DepthAmplitudeChannel)channel);
  tracker->SetLocation(point.x, point.y);

  // add a marker to image view
  auto markerItem = std::make_shared<CrossHairMarker>(point, name);
  markerItem->EnableLabel(markersDecorated);
  auto imageItem = view_->imageItems_[channel];
  imageItem->addChild(markerItem);

  // add a reference of the marker to the inspector tool, so it can move the
  // marker around
  tracker->graphicsItem = markerItem;
  children.push_back(tracker);
}