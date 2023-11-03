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

GraphicImageItem::GraphicImageItem(std::string name, GraphicsItem* parent)
    : GraphicsItem(name, parent) {
  imageSize_ = ImVec2(0, 0);
  geometries_.push_back(ImVec2(0, 0));
  geometries_.push_back(ImVec2(0, 0));
  uv0 = ImVec2(0.0f, 0.0f);
  uv1 = ImVec2(1.0f, 1.0f);
}

GraphicImageItem::GraphicImageItem(cv::Mat& image, std::string name,
                                   GraphicsItem* parent)
    : GraphicImageItem(name, parent) {
  setImage(image);
}

void GraphicImageItem::setImage(cv::Mat& image) {
  UploadCvMatToGpuTexture(image, (GLuint*)&imageTextureId_, &imageSize_);
  if (imageSize_ != geometries_[1]) {
    geometries_[1] = imageSize_;
    update();
  }
}

void GraphicImageItem::paintSelf() {
  if (imageTextureId_ == 0) {
    return;
  }
  ImVec2 pmin = sceneGeometries_[0];
  ImVec2 pmax = sceneGeometries_[1];
  ImGui::GetWindowDrawList()->AddImage(imageTextureId_, pmin, pmax, uv0, uv1);
}

void GraphicImageItem::clipSelf(ImRect r) {
  bool ret;
  ImVec2 pmin = sceneGeometries_[0];
  ImVec2 pmax = sceneGeometries_[1];
  ImVec2 size = pmax - pmin;
  ImRect rect = ImRect(pmin, pmax);
  rect.ClipWithFull(r);
  uv0 = (rect.Min - pmin) / size;
  uv1 = (rect.Max - pmin) / size;
  sceneGeometries_[0] = rect.Min;
  sceneGeometries_[1] = rect.Max;
}

bool GraphicImageItem::hitTest(ImVec2 p) {
  if (sceneGeometries_.size() < 2) {
    return false;
  }
  ImVec2 pmin = sceneGeometries_[0];
  ImVec2 pmax = sceneGeometries_[1];
  return ImRect(pmin, pmax).Contains(p);
}

void GraphicImageItem::setFitMode(FitMode mode) {}

GraphicTextItem::GraphicTextItem(std::string name, GraphicsItem* parent)
    : GraphicsItem(name, parent) {
  geometries_.push_back(ImVec2(0, 0));
}

GraphicTextItem::GraphicTextItem(std::string text, std::string name,
                                 GraphicsItem* parent)
    : GraphicTextItem(name, parent) {
  setText(text);
  setCorner(0);
}

void GraphicTextItem::setText(std::string text) { text_ = text; }

/**
 * @brief put the anchor of text box at corner number 0/1/2/3, 0 starts from
 * topleft and go clockwise..
 *
 * @param corner
 */
void GraphicTextItem::setCorner(int corner) { corner_ = corner % 4; }

void GraphicTextItem::setBackgroud(bool enable) { background_ = enable; }

void GraphicTextItem::paintSelf() {
  ImVec2 p = sceneGeometries_[0];
  ImVec2 size = ImGui::CalcTextSize(text_.c_str());
  ImVec2 displayPos;
  switch (corner_) {
    case 0:
      displayPos = p;
      break;
    case 1:
      displayPos = p + ImVec2(-size.x, 0);
      break;
    case 2:
      displayPos = p + ImVec2(-size.x, -size.y);
      break;
    case 3:
      displayPos = p + ImVec2(0, -size.y);
      break;
    default:
      break;
  }
  lastRect_ = ImRect(displayPos, displayPos + size);
  if (background_) {
    ImGui::GetWindowDrawList()->AddRectFilled(displayPos, displayPos + size,
                                              fillColor_, 0, 0);
  }
  ImGui::GetWindowDrawList()->AddText(displayPos, lineColor_, text_.c_str());
}

void GraphicTextItem::clipSelf(ImRect r) {
  // do nothing until we came up with a way to clip text
}

bool GraphicTextItem::hitTest(ImVec2 p) { return lastRect_.Contains(p); }
