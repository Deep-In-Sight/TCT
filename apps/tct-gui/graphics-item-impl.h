#pragma once
#include "graphics-item.h"

struct GraphicLineItem : public GraphicsItem {
  GraphicLineItem(ImVec2 p1, ImVec2 p2, std::string name = "",
                  GraphicsItem* parent = nullptr);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
  // make a rectangle and 2 semicircles at both ends wrapping around the line
  // for easy hit test
  int hitTestMargin_ = 1;
};

struct GraphicRectItem : public GraphicsItem {
  GraphicRectItem(ImVec2 p1, ImVec2 p2, std::string name = "",
                  GraphicsItem* parent = nullptr);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
};

struct GraphicPolygonItem : public GraphicsItem {
  GraphicPolygonItem(std::vector<ImVec2> points, std::string name = "",
                     GraphicsItem* parent = nullptr);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
};