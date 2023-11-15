#include "inspector-2d.h"

#include <iostream>
#include <opencv2/imgcodecs.hpp>

#include "graphics-item-impl.h"
#include "graphics-layout.h"
#include "inspector-graphics-view.h"
#include "utility.h"

Inspector2D::Inspector2D() {
  scene_ = std::make_shared<GraphicsScene>();
  view_ = std::make_shared<InspectorGraphicsView>(scene_);
  auto image = cv::imread("./data/images/MyImage01.jpg");
  view_->setImage(image);

  firstFrame = true;
  imageSizeChanged = false;
  windowChanged = false;
}

Inspector2D::~Inspector2D() {}

void Inspector2D::ImGuiDraw() {
  static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar |
                                        ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoScrollWithMouse;

  bool needRelayout =
      firstFrame || imageSizeChanged || windowChanged || view_->layoutChanged_;
  static ImVec2 lastWindowPos = ImVec2(0.0f, 0.0f);
  static ImVec2 lastWindowSize = ImVec2(0.0f, 0.0f);
  static ImVec2 lastMousePos = ImVec2(0.0f, 0.0f);

  if (ImGui::Begin("Inspector 2D", nullptr, windowFlags)) {
    if (needRelayout) {
      ImGuiLayout();
    }

    // draw menus
    DrawMenu();

    // child widgets draw
    view_->ImGuiDraw();

    // handle events
    if (ImGui::IsWindowHovered()) {
      handleMouse();
    }

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
}

void Inspector2D::ImGuiLayout() {
  contentRect = ImRect(ImGui::GetWindowContentRegionMin(),
                       ImGui::GetWindowContentRegionMax());
  view_->contentRect = contentRect;
  view_->ImGuiLayout();
}

void Inspector2D::DrawMenu() {
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
      if (ImGui::MenuItem("Zoom In", nullptr, nullptr)) {
        // imageWidget->setZoom(0.1f);
      }
      if (ImGui::MenuItem("Zoom Out", nullptr, nullptr)) {
        // imageWidget->setZoom(-0.1f);
      }
      ImGui::Separator();
      bool rulersEnabled = view_->rulersEnabled_;
      if (ImGui::MenuItem("Rulers", nullptr, &rulersEnabled)) {
        view_->enableRulers(rulersEnabled);
      }
      bool sideBySide = view_->sideBySide_;
      if (ImGui::MenuItem("Side By Side", nullptr, &sideBySide)) {
        view_->enableSideBySide(sideBySide);
      }

      ImGui::EndMenu();  // end menu View
    }
    ImGui::EndMenuBar();
  }
}

void Inspector2D::handleMouse() {
  auto& io = ImGui::GetIO();

  if (io.WantCaptureMouse) {
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