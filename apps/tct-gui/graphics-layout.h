#pragma once

#include "graphics-item.h"

/**
 * @brief simple layout to automatically position the children layout.
 * set margin and preferred size to control the layout.
 * if preferred size is not set, it will be calculated from the children.
 * size = margin + children's size
 */
struct GraphicsLayout : public GraphicsItem {
  GraphicsLayout(const std::string& name = "");
  virtual void layout();
  // left, top, right, bottom
  ImVec4 margins_;
  // width, height
  ImVec2 preferredSize_;
};

struct GraphicsHBoxLayout : public GraphicsLayout {
  GraphicsHBoxLayout(const std::string& name = "");
  void layout() override;
};

struct GraphicsVBoxLayout : public GraphicsLayout {
  GraphicsVBoxLayout(const std::string& name = "");
  void layout() override;
};
