#pragma once

#include <imgui_internal.h>

#include <memory>
#include <vector>

struct ImGuiWidget {
  /**
   * @brief call the ImGui functions to draw the widget and its children
   *
   */
  virtual void ImGuiDraw() = 0;
  /**
   * @brief call all child widgets' ImGuiGetPreferedSize to calculate the
   * contentRect of this widget.
   */
  virtual void ImGuiLayout() = 0;

  // virtual ImRect ImGuiGetPreferedSize() = 0;

  // contentRect of a widget is all absolute coordinates, origin is top-left of
  // window it belong to. (as oppose to relative to its immediate parent)
  ImRect contentRect;
  std::vector<std::shared_ptr<ImGuiWidget>> children;
};