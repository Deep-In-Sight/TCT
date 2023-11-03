#include "graphics-view.h"

#include "utility.h"

GraphicsScene::GraphicsScene() { rootItem_ = new GraphicsItem(); }

GraphicsScene::~GraphicsScene() { delete rootItem_; }

void GraphicsScene::addItem(GraphicsItem* item) { rootItem_->addChild(item); }

void GraphicsScene::paint() { rootItem_->paint(); }

GraphicsItem* GraphicsScene::itemAt(ImVec2 p) { return findItem(rootItem_, p); }

GraphicsView::GraphicsView(GraphicsScene* scene, bool enableDebug) {
  scene_ = scene;
  debug_ = enableDebug;
}

GraphicsView::~GraphicsView() {}

void GraphicsView::lookAt(ImVec2 p) { lookAt_ = p; }

void GraphicsView::moveViewTo(ImVec2 p) {
  auto windowPos = ImGui::GetWindowPos();
  auto viewPos = contentRect.Min + windowPos;
  auto viewSize = contentRect.GetSize();
  // center the p in the view
  auto pos = viewPos + viewSize * 0.5f - p;
  scene_->rootItem_->setPos(pos);
}

void nodeEdit(GraphicsItem* item, int level = 0) {
  char name[50];
  for (int i = 0; i < level; i++) {
    name[i] = '\t';
  }
  if (item->name_.empty()) {
    sprintf(name, "root");
  } else {
    sprintf(name + level, "%s", item->name_.c_str());
  }
  ImGui::SeparatorText(name);
  ImGui::BeginChild(name, ImVec2(0, 250));
  ImVec2 pos = item->pos_;
  ImVec2 origin = item->origin_;
  ImVec2 scale = ImVec2(item->T_.sx_, item->T_.sy_);
  ImVec2 translate = ImVec2(item->T_.dx_, item->T_.dy_);
  ImVec2 clipRectMin = item->clipRect_.Min;
  ImVec2 clipRectMax = item->clipRect_.Max;
  ImVec4 fillColor = item->fillColor_;
  ImVec4 lineColor = item->lineColor_;

  if (ImGui::DragFloat2("pos", &pos.x)) {
    item->setPos(pos);
  }
  if (ImGui::DragFloat2("origin", &origin.x)) {
    item->setOrigin(origin);
  }
  if (ImGui::DragFloat2("scale", &scale.x, 0.1f, 0.0f, 100.0f)) {
    item->scale(scale.x, scale.y);
  }
  if (ImGui::DragFloat2("translate", &translate.x)) {
    item->translate(translate.x, translate.y);
  }
  if (ImGui::DragFloat2("clipRectMin", &clipRectMin.x) ||
      ImGui::DragFloat2("clipRectMax", &clipRectMax.x)) {
    item->clip(ImRect(clipRectMin, clipRectMax));
  }
  if (ImGui::ColorEdit4("fillColor", &fillColor.x)) {
    item->fillColor_ = ImColor(fillColor);
  }
  if (ImGui::ColorEdit4("lineColor", &lineColor.x)) {
    item->lineColor_ = ImColor(lineColor);
  }

  if (ImGui::Button("Reset")) {
    item->setPos(ImVec2(0.0f, 0.0f));
    item->setOrigin(ImVec2(0.0f, 0.0f));
    item->scale(1.0f, 1.0f);
    item->translate(0.0f, 0.0f);
    item->unclip();
  }

  ImGui::EndChild();
  for (auto child : item->children_) {
    nodeEdit(child, level + 1);
  }
};

void GraphicsView::ImGuiDraw() {
  static ImVec2 lastPos, lastSize, lastLook;
  bool relayout = true;
  if (ImGui::Begin("GraphicsView")) {
    if (lastPos != ImGui::GetWindowPos() ||
        lastSize != ImGui::GetWindowSize() || lastLook != lookAt_) {
      lastPos = ImGui::GetWindowPos();
      lastSize = ImGui::GetWindowSize();
      lastLook = lookAt_;
      ImGuiLayout();
    }
    scene_->paint();
  }
  ImGui::End();

  if (debug_) {
    if (ImGui::Begin("GraphicsViewEditor")) {
      ImGui::DragFloat2("lookAt", &lookAt_.x);
      nodeEdit(scene_->rootItem_);
    }
    ImGui::End();
  }
}

void GraphicsView::ImGuiLayout() {
  contentRect = ImRect(ImGui::GetWindowContentRegionMin(),
                       ImGui::GetWindowContentRegionMax());
  moveViewTo(lookAt_);
}