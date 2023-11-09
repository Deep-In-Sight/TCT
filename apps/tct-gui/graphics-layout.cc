#include "graphics-layout.h"

GraphicsLayout::GraphicsLayout(const std::string& name)
    : GraphicsItem(name), margins_(0, 0, 0, 0), preferredSize_(0, 0) {}
void GraphicsLayout::layout() {}

GraphicsHBoxLayout::GraphicsHBoxLayout(const std::string& name)
    : GraphicsLayout(name) {}

void GraphicsHBoxLayout::layout() {
  ImVec2 pos(margins_.x, margins_.y);
  float maxChildHeight = 0;
  for (auto child : children_) {
    auto childLayout = dynamic_cast<GraphicsLayout*>(child.get());
    if (childLayout != nullptr) {
      // layout the children inside first
      childLayout->layout();
      childLayout->setPos(pos);
      childLayout->clip(ImRect(pos, pos + childLayout->preferredSize_));
      if (childLayout->preferredSize_.y > maxChildHeight) {
        maxChildHeight = childLayout->preferredSize_.y;
      }
      pos = pos + ImVec2(childLayout->preferredSize_.x, 0.0f);
    }
  }

  if (preferredSize_.x == 0) {
    preferredSize_.x = pos.x + margins_.z;
    preferredSize_.y = pos.y + maxChildHeight + margins_.w;
  }
}

GraphicsVBoxLayout::GraphicsVBoxLayout(const std::string& name)
    : GraphicsLayout(name) {}

void GraphicsVBoxLayout::layout() {
  ImVec2 pos(margins_.x, margins_.y);
  float maxChildWidth = 0;
  for (auto child : children_) {
    auto childLayout = dynamic_cast<GraphicsLayout*>(child.get());
    if (childLayout != nullptr) {
      // layout the children inside first
      childLayout->layout();
      childLayout->setPos(pos);
      pos = pos + ImVec2(0.0f, childLayout->preferredSize_.y);
      if (childLayout->preferredSize_.x > maxChildWidth) {
        maxChildWidth = childLayout->preferredSize_.x;
      }
    }
  }

  if (preferredSize_.y == 0) {
    preferredSize_.x = pos.x + maxChildWidth + margins_.z;
    preferredSize_.y = pos.y + margins_.w;
  }
}