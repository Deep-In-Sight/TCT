#include "inspector-2d.h"

Inspector2D::Inspector2D() {
  imageWidget = std::make_shared<ImageWidget>();
  firstFrame = true;
}

Inspector2D::~Inspector2D() {}

void Inspector2D::ImGuiDraw() {
  auto io = ImGui::GetIO();
  static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar |
                                        ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoScrollWithMouse;

  if (ImGui::Begin("Inspector 2D", nullptr, windowFlags)) {
    if (firstFrame) {
      firstFrame = false;
      contentRect = ImRect(ImGui::GetWindowContentRegionMin(),
                           ImGui::GetWindowContentRegionMax());
      imageWidget->contentRect = contentRect;
    }

    // draw menus
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("View")) {
        if (ImGui::BeginMenu("View Mode")) {
          static bool menuOriginalSelected = true;
          static bool menuFitSelected = false;
          static bool menuStretchSelected = false;
          ViewMode vm;
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
        ImGui::EndMenu();  // end menu View
      }
      ImGui::EndMenuBar();
    }

    // child widgets draw
    imageWidget->ImGuiDraw();

    // handle events
    if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
      float zoomStep = (io.MouseWheel > 0.0f) ? 0.1f : -0.1f;
      ImVec2 mousePos = io.MousePos - ImGui::GetWindowPos();
      imageWidget->setZoom(zoomStep, mousePos);
    }
  }
  ImGui::End();
}

void Inspector2D::ImGuiLayout() {
  contentRect = ImRect(ImGui::GetWindowContentRegionMin(),
                       ImGui::GetWindowContentRegionMax());
}