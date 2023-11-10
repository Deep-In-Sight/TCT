#pragma once

#include "graphics-item.h"

/**
 * @brief simple layout to automatically position the children layout.
 * set margin and preferred size to control the layout.
 * if preferred size is not set, it will be calculated from the children.
 * size = margin + children's size
 *
 * TODO: add support for stretch
 */
struct GraphicsLayout : public GraphicsItem {
  GraphicsLayout(const std::string& name = "",
                 ImVec2 preferredSize = ImVec2(0, 0),
                 ImVec4 margins = ImVec4(0, 0, 0, 0));
  virtual void layout();
  std::string printLayout();
  // left, top, right, bottom
  ImVec4 margins_;
  // width, height
  ImVec2 preferredSize_;
};

struct GraphicsHBoxLayout : public GraphicsLayout {
  GraphicsHBoxLayout(const std::string& name = "",
                     ImVec2 preferredSize = ImVec2(0, 0),
                     ImVec4 margins = ImVec4(0, 0, 0, 0));
  void layout() override;
};

struct GraphicsVBoxLayout : public GraphicsLayout {
  GraphicsVBoxLayout(const std::string& name = "",
                     ImVec2 preferredSize = ImVec2(0, 0),
                     ImVec4 margins = ImVec4(0, 0, 0, 0));
  void layout() override;
};

/**
 * @brief child layout are added in row major order, aka left to right, top to
 * bottom. If you add more than rows * cols children, the extra child layouts
 * will be ignored, automatically position at top left of the grid.
 *
 */
struct GraphicsGridLayout : public GraphicsLayout {
  GraphicsGridLayout(int rows, int cols, const std::string& name = "",
                     ImVec2 preferredSize = ImVec2(0, 0),
                     ImVec4 margins = ImVec4(0, 0, 0, 0));
  void layout() override;
  int rows_;
  int cols_;
};