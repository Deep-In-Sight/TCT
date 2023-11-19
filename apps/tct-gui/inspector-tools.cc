#include "inspector-tools.h"

#include <imgui.h>

HLineScannerView::HLineScannerView(const std::string& name)
    : ImGuiWidget(name) {}

void HLineScannerView::ImGuiLayout() {}

void HLineScannerView::ImGuiDraw() {
  ImGui::Begin(name_.c_str());
  ImGui::Text("HLineScannerView");
  ImGui::End();
}
