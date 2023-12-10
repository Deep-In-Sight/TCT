#include "graphics-item-impl.h"

#include <iostream>

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
  if (image.empty()) {
    throw std::runtime_error("empty image");
  }
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
  geometries_.push_back(pos);
  setText(text);
  setAnchor(0);
  setBackgroud(false);
  setFontScale(1.0f);
}

void GraphicTextItem::setText(std::string text) { text_ = text; }

/**
 * @brief put the anchor of text box at corner number 0/1/2/3/4/5/6/7/8
 *
 * 0────1─────2
 * │          │
 * 3 te[4]xt  5
 * │          │
 * 6────7─────8
 *
 * @param corner
 */
void GraphicTextItem::setAnchor(int anchor) { anchor_ = anchor % 8; }

void GraphicTextItem::setBackgroud(bool enable) { background_ = enable; }

void GraphicTextItem::setFontScale(float scale) { fontScale_ = scale; }

void GraphicTextItem::paintSelf() {
  if (sceneGeometries_.empty()) return;
  ImVec2 size = ImGui::CalcTextSize(text_.c_str());
  int w = size.x, h = size.y;
  int c = anchor_;
  float x = (c % 3 == 0) ? 0 : (c % 3 == 1) ? -w / 2 : -w;
  float y = (c / 3 == 0) ? 0 : (c / 3 == 1) ? -h / 2 : -h;
  bb_ = ImRect(x, y, x + w, y + h);
  bb_.Translate(sceneGeometries_[0]);
  if (background_) {
    ImGui::GetWindowDrawList()->AddRectFilled(bb_.Min, bb_.Max, fillColor_, 0,
                                              0);
  }
  ImGui::GetWindowDrawList()->AddText(bb_.Min, lineColor_, text_.c_str());
}

void GraphicTextItem::clipSelf(ImRect r) {
  // do nothing until we came up with a way to clip text
  if (!r.Contains(sceneGeometries_[0])) {
    sceneGeometries_.clear();
  }
}

bool GraphicTextItem::hitTest(ImVec2 p) { return bb_.Contains(p); }

void GraphicTextItem::paintBegin() {
  oldFontScale_ = ImGui::GetFont()->Scale;
  ImGui::GetFont()->Scale = fontScale_;
  ImGui::PushFont(ImGui::GetFont());
}

void GraphicTextItem::paintEnd() {
  ImGui::GetFont()->Scale = oldFontScale_;
  ImGui::PopFont();
}

Ruler::Ruler(bool horizontal, int min, int max, int major, int minor,
             float size, std::string name)
    : GraphicsItem(name), orientation_(horizontal) {
  // create ruler items
  ImRect bg =
      (horizontal) ? ImRect(min, 0, max, size) : ImRect(0, min, size, max);
  auto bgItem = std::make_shared<GraphicRectItem>(bg.Min, bg.Max, name);
  auto fgItem = std::make_shared<GraphicsItem>();
  addChild(bgItem);
  addChild(fgItem);

  ImRect line = (horizontal) ? ImRect(min, 0, max, 0) : ImRect(0, min, 0, max);
  auto lineItem = std::make_shared<GraphicLineItem>(line.Min, line.Max);
  auto tickGroup = std::make_shared<GraphicsItem>();
  auto textGroup = std::make_shared<GraphicsItem>();
  fgItem->addChild(lineItem);
  fgItem->addChild(tickGroup);
  fgItem->addChild(textGroup);

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
  highlightItem_ =
      std::make_shared<GraphicLineItem>(highlightTick.Min, highlightTick.Max);
  tickGroup->addChild(highlightItem_);

  // coloring
  for (auto item : tickGroup->children_) {
    item->lineColor_ = tickColor_;
  }
  for (auto item : textGroup->children_) {
    item->lineColor_ = textColor_;
  }
  bgItem->fillColor_ = backgroundColor_;
  bgItem->lineColor_ = ImColor(0, 0, 0, 0);
  lineItem->lineColor_ = lineColor_;
  highlightItem_->lineColor_ = highlightColor_;

  // positioning
  ImVec2 linePos =
      (horizontal) ? ImVec2(0, size - padding) : ImVec2(size - padding, 0);
  lineItem->setPos(linePos);
  tickGroup->setPos(linePos);

  fgItem->isClickable_ = false;
}

void Ruler::highlight(int value) {
  ImVec2 pos = (orientation_) ? ImVec2(value, 0) : ImVec2(0, value);
  highlightItem_->setPos(pos);
}

Ruler::~Ruler() {}

void Ruler::paintBegin() {
  oldFontScale_ = ImGui::GetFont()->Scale;
  ImGui::GetFont()->Scale *= 0.7;
  ImGui::PushFont(ImGui::GetFont());
}

void Ruler::paintEnd() {
  ImGui::GetFont()->Scale = oldFontScale_;
  ImGui::PopFont();
}

CrossHairItem::CrossHairItem(const std::string& name, ImVec2 pos)
    : GraphicsItem(name), pos_(pos) {
  geometries_.push_back(ImVec2(0, 0));

  lineColor_ = color_;
  lineWidth_ = thickness_;
}

void CrossHairItem::paintSelf() {
  ImVec2 p = sceneGeometries_[0];
  auto pMin = p - ImVec2(sizeDiv2_, sizeDiv2_);
  auto pMax = p + ImVec2(sizeDiv2_, sizeDiv2_);
  ImRect r(pMin, pMax);

  auto p1 = (r.GetTL() + r.GetTR()) / 2;
  auto p2 = (r.GetBL() + r.GetBR()) / 2;
  auto p3 = (r.GetTL() + r.GetBL()) / 2;
  auto p4 = (r.GetTR() + r.GetBR()) / 2;

  ImGui::GetWindowDrawList()->AddRect(r.Min, r.Max, lineColor_, 0, 0,
                                      lineWidth_);
  ImGui::GetWindowDrawList()->AddLine(p1, p2, lineColor_, lineWidth_);
  ImGui::GetWindowDrawList()->AddLine(p3, p4, lineColor_, lineWidth_);
}

void CrossHairItem::clipSelf(ImRect r) {
  ImRect rect(sceneGeometries_[0], sceneGeometries_[1]);
  rect.ClipWithFull(r);
  sceneGeometries_[0] = rect.Min;
  sceneGeometries_[1] = rect.Max;
}

bool CrossHairItem::hitTest(ImVec2 p) {
  ImRect rect(sceneGeometries_[0], sceneGeometries_[1]);
  return rect.Contains(p);
}

InspectorMarker::InspectorMarker(const std::string& name)
    : GraphicsItem(name) {}

void InspectorMarker::AddLabel(std::string text, ImVec2 pos, int anchor) {
  auto label = std::make_shared<GraphicTextItem>(text, pos);
  label->anchor_ = anchor;
  label->background_ = true;
  label->lineColor_ = ImColor(255, 255, 255, 255);
  label->fillColor_ = ImColor(0, 0, 0, 255);
  labels.push_back(label);
  addChild(label);
}

void InspectorMarker::UpdateLabel(int labelIndex, ImVec2 pos,
                                  const std::string& text) {
  auto label = labels[labelIndex];
  label->geometries_[0] = pos;
  update();
  label->setText(text);
}

void InspectorMarker::EnableLabel(bool enable) {
  if (enable) {
    for (auto item : labels) {
      item->isHidden_ = false;
    }
  } else {
    for (auto item : labels) {
      item->isHidden_ = true;
    }
  }
  update();
}

std::string toString(ImVec2 p) {
  return std::to_string((int)p.x) + "," + std::to_string((int)p.y);
}

LineMarker::LineMarker(ImVec2 p1, ImVec2 p2, const std::string& name)
    : InspectorMarker(name) {
  auto line = std::make_shared<GraphicLineItem>(p1, p2);
  line->lineColor_ = ImColor(0, 255, 0, 255);
  line->lineWidth_ = 2.0f;
  addChild(line);
  AddLabel(toString(p1), p1, 4);
  AddLabel(toString(p2), p2, 4);
  AddLabel(name, (p1 + p2) / 2, 4);
}

void LineMarker::modify(ImVec2 p1, ImVec2 p2) {
  auto lineItem = std::dynamic_pointer_cast<GraphicLineItem>(children_[0]);
  lineItem->geometries_[0] = p1;
  lineItem->geometries_[1] = p2;
  UpdateLabel(0, p1, toString(p1));
  UpdateLabel(1, p2, toString(p2));
  UpdateLabel(2, (p1 + p2) / 2, name_);
  update();
}

RectMarker::RectMarker(ImVec2 pmin, ImVec2 pmax, const std::string& name)
    : InspectorMarker(name) {
  auto rect = std::make_shared<GraphicRectItem>(pmin, pmax);
  rect->fillColor_ = ImColor(0, 255, 0, 80);
  rect->lineColor_ = ImColor(0, 255, 0, 255);
  rect->lineWidth_ = 2.0f;
  addChild(rect);
  auto l1 = std::to_string((int)pmin.x) + "," + std::to_string((int)pmin.y);
  auto l2 = std::to_string((int)pmax.x) + "," + std::to_string((int)pmax.y);
  AddLabel(l1, pmin, 4);
  AddLabel(l2, pmax, 4);
  AddLabel(name, (pmin + pmax) / 2, 4);
}

void RectMarker::modify(ImVec2 p1, ImVec2 p2) {
  auto rectItem = std::dynamic_pointer_cast<GraphicRectItem>(children_[0]);
  rectItem->geometries_[0] = p1;
  rectItem->geometries_[1] = p2;
  UpdateLabel(0, p1, toString(p1));
  UpdateLabel(1, p2, toString(p2));
  UpdateLabel(2, (p1 + p2) / 2, name_);
  update();
}

CrossHairMarker::CrossHairMarker(ImVec2 pos, const std::string& name)
    : InspectorMarker(name) {
  auto crossHair = std::make_shared<CrossHairItem>(name, pos);
  addChild(crossHair);

  AddLabel("", pos, 6);
  AddLabel(name, pos, 0);
  UpdateLabel(0, pos, toString(pos));
}

void CrossHairMarker::modify(ImVec2 pos) {
  auto crossHair = std::dynamic_pointer_cast<CrossHairItem>(children_[0]);
  crossHair->setPos(pos);
  UpdateLabel(0, pos, toString(pos));
  UpdateLabel(1, pos, name_);
}