#pragma once
#include "graphics-item.h"
#include "imgui-widget.h"

struct GraphicsScene {
  GraphicsScene();
  ~GraphicsScene();

  void addItem(GraphicsItemPtr item);
  void paint();
  GraphicsItemPtr itemAt(ImVec2 p);

  GraphicsItemPtr rootItem_;
};

struct GraphicsView : public ImGuiWidget {
  GraphicsView(GraphicsScene* scene, bool enableDebug = false);
  ~GraphicsView();

  void lookAt(ImVec2 p);
  void moveViewTo(ImVec2 p);

  void ImGuiDraw() override;
  void ImGuiLayout() override;

  GraphicsScene* scene_;
  ImVec2 lookAt_;
  bool debug_;
};