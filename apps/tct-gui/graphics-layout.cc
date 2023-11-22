#include "graphics-layout.h"

#include <functional>
#include <iostream>
#include <sstream>

GraphicsLayout::GraphicsLayout(const std::string& name, ImVec2 preferredSize,
                               ImVec4 margins)
    : GraphicsItem(name), margins_(margins), preferredSize_(preferredSize) {
  isFixedSize_ = (preferredSize.x > 0 || preferredSize.y > 0);
}

void GraphicsLayout::layout() {}

std::string GraphicsLayout::toString() {
  std::function<void(GraphicsLayout*, int, std::ostringstream&)>
      printLayoutRecursive = [&](GraphicsLayout* layout, int depth,
                                 std::ostringstream& layoutStr) {
        std::string leading;
        GraphicsLayout* current = layout;
        GraphicsLayout* parent =
            dynamic_cast<GraphicsLayout*>(current->parent_);
        bool isLast = true;
        if (parent != nullptr) {
          isLast = (current == parent->children_.back().get());
        }
        leading = (isLast) ? "  └►[" : "  ├►[";
        while (parent != nullptr) {
          GraphicsLayout* grandparent =
              dynamic_cast<GraphicsLayout*>(parent->parent_);
          bool parentIsLast = true;
          if (grandparent != nullptr) {
            parentIsLast = (parent == grandparent->children_.back().get());
          }
          if (parentIsLast) {
            leading = "   " + leading;
          } else {
            leading = "  |" + leading;
          }
          parent = grandparent;
        }

        std::string type;
        if (dynamic_cast<GraphicsHBoxLayout*>(layout) != nullptr) {
          type = "HBox";
        } else if (dynamic_cast<GraphicsVBoxLayout*>(layout) != nullptr) {
          type = "VBox";
        } else if (dynamic_cast<GraphicsGridLayout*>(layout) != nullptr) {
          type = "Grid";
        } else {
          type = "Layout";
        }

        layoutStr << leading << type << "]: " << layout->name_ << " pos: ("
                  << layout->pos_.x << "," << layout->pos_.y
                  << "), preferredSize: (" << layout->preferredSize_.x << ","
                  << layout->preferredSize_.y << ")" << std::endl;

        for (auto childit = layout->children_.begin();
             childit != layout->children_.end(); ++childit) {
          auto childLayout = dynamic_cast<GraphicsLayout*>(childit->get());
          if (childLayout != nullptr) {
            printLayoutRecursive(childLayout, depth + 1, layoutStr);
          }
        }
      };

  std::ostringstream layoutStr;
  printLayoutRecursive(this, 0, layoutStr);

  return layoutStr.str();
}

GraphicsHBoxLayout::GraphicsHBoxLayout(const std::string& name,
                                       ImVec2 preferredSize, ImVec4 margins)
    : GraphicsLayout(name, preferredSize, margins) {}

void GraphicsHBoxLayout::layout() {
  ImVec2 pos(margins_.x, margins_.y);
  float maxChildHeight = 0;
  for (auto child : children_) {
    auto childLayout = dynamic_cast<GraphicsLayout*>(child.get());
    if (childLayout != nullptr && !childLayout->isHidden_) {
      // layout the children inside first
      childLayout->layout();
      childLayout->setPos(pos);
      childLayout->clip(ImRect(ImVec2(0, 0), childLayout->preferredSize_));
      if (childLayout->preferredSize_.y > maxChildHeight) {
        maxChildHeight = childLayout->preferredSize_.y;
      }
      pos = pos + ImVec2(childLayout->preferredSize_.x, 0.0f);
    }
  }

  if (!isFixedSize_) {
    preferredSize_.x = pos.x + margins_.z;
    preferredSize_.y = pos.y + maxChildHeight + margins_.w;
  }
}

GraphicsVBoxLayout::GraphicsVBoxLayout(const std::string& name,
                                       ImVec2 preferredSize, ImVec4 margins)
    : GraphicsLayout(name, preferredSize, margins) {}

void GraphicsVBoxLayout::layout() {
  ImVec2 pos(margins_.x, margins_.y);
  float maxChildWidth = 0;
  for (auto child : children_) {
    auto childLayout = dynamic_cast<GraphicsLayout*>(child.get());
    if (childLayout != nullptr && !childLayout->isHidden_) {
      // layout the children inside first
      childLayout->layout();
      childLayout->setPos(pos);
      childLayout->clip(ImRect(ImVec2(0, 0), childLayout->preferredSize_));
      if (childLayout->preferredSize_.x > maxChildWidth) {
        maxChildWidth = childLayout->preferredSize_.x;
      }
      pos = pos + ImVec2(0.0f, childLayout->preferredSize_.y);
    }
  }

  if (!isFixedSize_) {
    preferredSize_.x = pos.x + maxChildWidth + margins_.z;
    preferredSize_.y = pos.y + margins_.w;
  }
}

GraphicsGridLayout::GraphicsGridLayout(int rows, int cols,
                                       const std::string& name,
                                       ImVec2 preferredSize, ImVec4 margins)
    : GraphicsLayout(name, preferredSize, margins), rows_(rows), cols_(cols) {}

void GraphicsGridLayout::layout() {
  float maxChildWidth[cols_];
  float maxChildHeight[rows_];
  // reset
  for (int col = 0; col < cols_; col++) {
    maxChildWidth[col] = 0;
  }
  for (int row = 0; row < rows_; row++) {
    maxChildHeight[row] = 0;
  }
  // layout the children first
  for (int row = 0; row < rows_; row++) {
    for (int col = 0; col < cols_; col++) {
      int index = row * cols_ + col;
      if (index >= children_.size()) {
        break;
      }
      auto child = children_[index].get();
      auto childLayout = dynamic_cast<GraphicsLayout*>(child);
      if (childLayout != nullptr && !childLayout->isHidden_) {
        childLayout->layout();
        if (childLayout->preferredSize_.y > maxChildHeight[row]) {
          maxChildHeight[row] = childLayout->preferredSize_.y;
        }
        if (childLayout->preferredSize_.x > maxChildWidth[col]) {
          maxChildWidth[col] = childLayout->preferredSize_.x;
        }
      }
    }
  }
  // now position the children
  ImVec2 rowPos(margins_.x, margins_.y);
  ImVec2 colPos;
  for (int row = 0; row < rows_; row++) {
    colPos = rowPos;
    for (int col = 0; col < cols_; col++) {
      int index = row * cols_ + col;
      if (index >= children_.size()) {
        break;
      }
      auto child = children_[index].get();
      auto childLayout = dynamic_cast<GraphicsLayout*>(child);
      if (childLayout != nullptr && !childLayout->isHidden_) {
        childLayout->setPos(colPos);
        childLayout->clip(ImRect(ImVec2(0, 0), childLayout->preferredSize_));
        colPos = colPos + ImVec2(maxChildWidth[col], 0.0f);
      }
    }
    rowPos = rowPos + ImVec2(0.0f, maxChildHeight[row]);
  }

  if (!isFixedSize_) {
    preferredSize_.x = colPos.x + margins_.z;
    preferredSize_.y = rowPos.y + margins_.w;
  }
}

std::shared_ptr<GraphicsLayout> GraphicsGridLayout::operator()(int row,
                                                               int col) {
  int index = row * cols_ + col;
  if (index >= children_.size()) {
    return nullptr;
  }
  return std::dynamic_pointer_cast<GraphicsLayout>(children_[index]);
}