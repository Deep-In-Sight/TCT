#include "graphics-item-impl.h"

#include "utility.h"

GraphicLineItem::GraphicLineItem(ImVec2 p1, ImVec2 p2, std::string name)
    : GraphicsItem(name) {
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

GraphicRectItem::GraphicRectItem(ImVec2 pmin, ImVec2 pmax, std::string name)
    : GraphicsItem(name) {
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
                                       std::string name)
    : GraphicsItem(name) {
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

GraphicImageItem::GraphicImageItem(std::string name) : GraphicsItem(name) {
  imageSize_ = ImVec2(0, 0);
  geometries_.push_back(ImVec2(0, 0));
  geometries_.push_back(ImVec2(0, 0));
  uv0 = ImVec2(0.0f, 0.0f);
  uv1 = ImVec2(1.0f, 1.0f);
}

GraphicImageItem::GraphicImageItem(cv::Mat& image, std::string name)
    : GraphicImageItem(name) {
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
  beforeClippedGeometries_ = sceneGeometries_;
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

ImVec2 GraphicImageItem::toOriginalUV(ImVec2 p) {
  auto pmin = beforeClippedGeometries_[0];
  auto pmax = beforeClippedGeometries_[1];
  auto size = pmax - pmin;
  return (p - pmin) / size * imageSize_;
}

GraphicTextItem::GraphicTextItem(std::string name)
    : GraphicTextItem("", ImVec2(0, 0), name) {}

GraphicTextItem::GraphicTextItem(std::string text, ImVec2 pos, std::string name)
    : GraphicsItem(name) {
  pos_ = pos;
  setText(text);
  setAnchor(0);
  setBackgroud(false);
}

void GraphicTextItem::setText(std::string text) { text_ = text; }

/**
 * @brief put the anchor of text box at corner number 0/1/2/3, 0 starts from
 * topleft and go clockwise..
 *
 * 0────1─────2
 * │          │
 * 7   text   3
 * │          │
 * 6────5─────4
 *
 * @param corner
 */
void GraphicTextItem::setAnchor(int anchor) { anchor_ = anchor % 8; }

void GraphicTextItem::setBackgroud(bool enable) { background_ = enable; }

void GraphicTextItem::paintSelf() {
  ImVec2 size = ImGui::CalcTextSize(text_.c_str());
  int w = size.x, h = size.y;
  int c = anchor_;
  float x = (c == 0 || c == 7 || c == 6) ? 0 : (c == 1 || c == 5) ? -w / 2 : -w;
  float y = (c == 0 || c == 1 || c == 2) ? 0 : (c == 3 || c == 7) ? -h / 2 : -h;
  bb_ = ImRect(x, y, x + w, y + h);
  if (background_) {
    ImGui::GetWindowDrawList()->AddRectFilled(bb_.Min, bb_.Max, fillColor_, 0,
                                              0);
  }
  ImGui::GetWindowDrawList()->AddText(bb_.Min, lineColor_, text_.c_str());
}

void GraphicTextItem::clipSelf(ImRect r) {
  // do nothing until we came up with a way to clip text
}

bool GraphicTextItem::hitTest(ImVec2 p) { return bb_.Contains(p); }

Ruler::Ruler(bool horizontal, int min, int max, int major, int minor,
             float size, std::string name)
    : GraphicsItem(name), orientation_(horizontal) {
  // create ruler items
  ImRect bg =
      (horizontal) ? ImRect(min, 0, max, size) : ImRect(0, min, size, max);
  ImRect line = (horizontal) ? ImRect(min, 0, max, 0) : ImRect(0, min, 0, max);
  auto bgItem = std::make_shared<GraphicRectItem>(bg.Min, bg.Max);
  auto lineItem = std::make_shared<GraphicLineItem>(line.Min, line.Max);
  addChild(bgItem);
  addChild(lineItem);

  auto tickGroup = std::make_shared<GraphicsItem>();
  auto textGroup = std::make_shared<GraphicsItem>();
  addChild(tickGroup);
  addChild(textGroup);

  for (int i = min; i <= max; i++) {
    if (i % major == 0) {
      ImRect tick = (horizontal) ? ImRect(i, 0, i, -majorSize_)
                                 : ImRect(0, i, -majorSize_, i);
      ImVec2 textPos = (horizontal) ? ImVec2(i, 0) : ImVec2(0, i);
      auto tickItem = std::make_shared<GraphicLineItem>(tick.Min, tick.Max);
      auto textItem =
          std::make_shared<GraphicTextItem>(std::to_string(i), textPos);
      tickGroup->addChild(tickItem);
      textGroup->addChild(textItem);
    } else if (i % minor == 0) {
      ImRect tick = (horizontal) ? ImRect(i, 0, i, -minorSize_)
                                 : ImRect(0, i, -minorSize_, i);
      auto tickItem = std::make_shared<GraphicLineItem>(tick.Min, tick.Max);
      tickGroup->addChild(tickItem);
    }
  }

  ImRect highlightTick = (horizontal) ? ImRect(0, 0, 0, -majorSize_)
                                      : ImRect(0, 0, -majorSize_, 0);
  auto highlightItem_ =
      std::make_shared<GraphicLineItem>(highlightTick.Min, highlightTick.Max);
  tickGroup->addChild(highlightItem_);

  // coloring
  for (auto item : tickGroup->children_) {
    item->lineColor_ = lineColor_;
  }
  for (auto item : textGroup->children_) {
    item->lineColor_ = lineColor_;
  }
  bgItem->fillColor_ = backgroundColor_;
  lineItem->lineColor_ = lineColor_;
  highlightItem_->lineColor_ = highlightColor_;

  // positioning
  ImVec2 linePos =
      (horizontal) ? ImVec2(0, size - padding) : ImVec2(size - padding, 0);
  lineItem->setPos(linePos);
  tickGroup->setPos(linePos);

  isClickable_ = false;
}

void Ruler::highlight(int value) {
  ImVec2 pos = (orientation_) ? ImVec2(value, 0) : ImVec2(0, value);
  highlightItem_->setPos(pos);
}

Ruler::~Ruler() {}