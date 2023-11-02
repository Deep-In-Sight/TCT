#include "graphics-item-impl.h"

#include "utility.h"

GraphicLineItem::GraphicLineItem(ImVec2 p1, ImVec2 p2, std::string name,
                                 GraphicsItem* parent)
    : GraphicsItem(name, parent) {
  geometries_.push_back(p1);
  geometries_.push_back(p2);
}

void GraphicLineItem::paintSelf() {
  if (sceneGeometries_.size() < 2) {
    return;
  }
  ImVec2 p0 = sceneGeometries_[0];
  ImVec2 p1 = sceneGeometries_[1];
  ImGui::GetWindowDrawList()->AddLine(p0, p1, lineColor_, lineWidth_);
}

void GraphicLineItem::clipSelf(ImRect r) {
  bool ret;
  ImVec2 p1 = sceneGeometries_[0];
  ImVec2 p2 = sceneGeometries_[1];
  ImVec2 p1_out, p2_out;
  ret = lineClip(p1, p2, r, p1_out, p2_out);
  if (ret) {
    sceneGeometries_[0] = p1_out;
    sceneGeometries_[1] = p2_out;
  } else {
    sceneGeometries_.clear();
  }
}

bool GraphicLineItem::hitTest(ImVec2 p) {
  if (sceneGeometries_.size() < 2) {
    return false;
  }
  ImVec2 p1 = sceneGeometries_[0];
  ImVec2 p2 = sceneGeometries_[1];
  ImVec2 closestPoint;
  // paranoid much?
  if (p1 == p2) {
    closestPoint = p1;
  } else {
    closestPoint = ImLineClosestPoint(p1, p2, p);
  }
  auto d = ImLengthSqr(p - closestPoint);
  return d <= hitTestMargin_ * hitTestMargin_;
}

GraphicRectItem::GraphicRectItem(ImVec2 pmin, ImVec2 pmax, std::string name,
                                 GraphicsItem* parent)
    : GraphicsItem(name, parent) {
  geometries_.push_back(pmin);
  geometries_.push_back(pmax);
}

void GraphicRectItem::paintSelf() {
  if (sceneGeometries_.size() < 2) {
    return;
  }
  ImVec2 pmin = sceneGeometries_[0];
  ImVec2 pmax = sceneGeometries_[1];
  ImGui::GetWindowDrawList()->AddRectFilled(pmin, pmax, fillColor_, 0, 0);
  ImGui::GetWindowDrawList()->AddRect(pmin, pmax, lineColor_, 0, 0, lineWidth_);
}

bool GraphicRectItem::hitTest(ImVec2 p) {
  if (sceneGeometries_.size() < 2) {
    return false;
  }
  ImVec2 pmin = sceneGeometries_[0];
  ImVec2 pmax = sceneGeometries_[1];
  return ImRect(pmin, pmax).Contains(p);
}

void GraphicRectItem::clipSelf(ImRect r) {
  bool ret;
  ImVec2 pmin = sceneGeometries_[0];
  ImVec2 pmax = sceneGeometries_[1];
  ImRect rect = ImRect(pmin, pmax);
  rect.ClipWithFull(r);
  sceneGeometries_[0] = rect.Min;
  sceneGeometries_[1] = rect.Max;
}

GraphicPolygonItem::GraphicPolygonItem(std::vector<ImVec2> points,
                                       std::string name, GraphicsItem* parent)
    : GraphicsItem(name, parent) {
  geometries_ = points;
}

void GraphicPolygonItem::paintSelf() {
  if (sceneGeometries_.size() < 3) {
    return;
  }
  ImGui::GetWindowDrawList()->AddConvexPolyFilled(
      sceneGeometries_.data(), sceneGeometries_.size(), fillColor_);
  ImGui::GetWindowDrawList()->AddPolyline(sceneGeometries_.data(),
                                          sceneGeometries_.size(), lineColor_,
                                          true, lineWidth_);
}

void GraphicPolygonItem::clipSelf(ImRect r) {
  sceneGeometries_ = polygonClip(sceneGeometries_, r);
}

bool GraphicPolygonItem::hitTest(ImVec2 p) {
  if (sceneGeometries_.size() < 3) {
    return false;
  }
  return polygonContain(sceneGeometries_, p);
}