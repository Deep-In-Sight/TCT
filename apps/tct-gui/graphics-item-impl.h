#pragma once
#include "graphics-item.h"

struct GraphicLineItem : public GraphicsItem {
  GraphicLineItem(ImVec2 p1, ImVec2 p2, std::string name = "",
                  GraphicsItem* parent = nullptr);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
};

struct GraphicRectItem : public GraphicsItem {
  GraphicRectItem(ImVec2 p1, ImVec2 p2, std::string name = "",
                  GraphicsItem* parent = nullptr);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
};

struct GraphicPolygonItem : public GraphicsItem {
  GraphicPolygonItem(std::vector<ImVec2> points, std::string name = "",
                     GraphicsItem* parent = nullptr);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
};