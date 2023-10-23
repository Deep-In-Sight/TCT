#include "inspector-2d.h"

#include "utility.h"

Inspector2D::Inspector2D() {
  imageWidget = std::make_shared<ImageWidget>();
  firstFrame = true;
  imageSizeChanged = false;
  windowSizeChanged = false;
}

Inspector2D::~Inspector2D() {}

void Inspector2D::ImGuiDraw() {
  auto io = ImGui::GetIO();
  static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar |
                                        ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoScrollWithMouse;

  bool needRelayout = firstFrame || imageSizeChanged || windowSizeChanged;
  static ImVec2 lastWindowSize = ImVec2(0.0f, 0.0f);
  static ImVec2 lastMousePos = ImVec2(0.0f, 0.0f);

  if (ImGui::Begin("Inspector 2D", nullptr, windowFlags)) {
    if (needRelayout) {
      ImGuiLayout();
    }

    // draw menus
    DrawMenu();

    // child widgets draw
    imageWidget->ImGuiDraw();

    // handle events
    if (ImGui::IsWindowHovered()) {
      ImVec2 mousePos = io.MousePos - ImGui::GetWindowPos();
      float mouseScroll = io.MouseWheel;

      if (mousePos != lastMousePos) {
        lastMousePos = mousePos;
        onMouseMove(mousePos);
      }

      if (mouseScroll != 0.0f) {
        onMouseScroll(mousePos, mouseScroll);
      }
    }

    if (firstFrame) {
      firstFrame = false;
    }

    if (ImGui::GetWindowSize() != lastWindowSize) {
      windowSizeChanged = true;
      lastWindowSize = ImGui::GetWindowSize();
    } else {
      windowSizeChanged = false;
    }
  }
  ImGui::End();
}

void Inspector2D::ImGuiLayout() {
  contentRect = ImRect(ImGui::GetWindowContentRegionMin(),
                       ImGui::GetWindowContentRegionMax());
  imageWidget->contentRect = contentRect;
  imageWidget->ImGuiLayout();
}

void Inspector2D::DrawMenu() {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("View")) {
      if (ImGui::BeginMenu("View Mode")) {
        static bool menuOriginalSelected = true;
        static bool menuFitSelected = false;
        static bool menuStretchSelected = false;
        ViewMode vm = imageWidget->viewMode;
        if (ImGui::MenuItem("Original", nullptr, &menuOriginalSelected)) {
          vm = ViewMode::kViewOriginal;
          imageWidget->setViewMode(vm);
        }
        if (ImGui::MenuItem("Fit", nullptr, &menuFitSelected)) {
          vm = ViewMode::kViewFit;
          imageWidget->setViewMode(vm);
        }
        if (ImGui::MenuItem("Stretch", nullptr, &menuStretchSelected)) {
          vm = ViewMode::kViewStretch;
          imageWidget->setViewMode(vm);
        }
        menuOriginalSelected = (vm == ViewMode::kViewOriginal);
        menuFitSelected = (vm == ViewMode::kViewFit);
        menuStretchSelected = (vm == ViewMode::kViewStretch);
        ImGui::EndMenu();  // end menu View / View Mode
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Zoom In", nullptr, nullptr)) {
        imageWidget->setZoom(0.1f);
      }
      if (ImGui::MenuItem("Zoom Out", nullptr, nullptr)) {
        imageWidget->setZoom(-0.1f);
      }
      ImGui::Separator();
      bool showRulers = imageWidget->showRulers;
      if (ImGui::MenuItem("Show Rulers", nullptr, &showRulers)) {
        imageWidget->setShowRulers(showRulers);
      }
      ImGui::EndMenu();  // end menu View
    }
    ImGui::EndMenuBar();
  }
}

void Inspector2D::onMouseMove(ImVec2 mousePos) {}

void Inspector2D::onMouseScroll(ImVec2 mousePos, float scroll) {
  if (imageWidget->contentRect.Contains(mousePos)) {
    imageWidget->onMouseScroll(mousePos, scroll);
  }
}