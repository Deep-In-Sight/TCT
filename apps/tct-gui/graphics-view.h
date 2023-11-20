#pragma once
#include "graphics-item.h"
#include "imgui-widget.h"

struct GraphicsScene {
  GraphicsScene();
  ~GraphicsScene();

  void addItem(GraphicsItemPtr item);
  void removeItem(GraphicsItemPtr item);
  void paint();
  GraphicsItemPtr itemAt(ImVec2 p);
  GraphicsItemPtr getItemByName(const std::string& name);

  GraphicsItemPtr rootItem_;
};

struct GraphicsView : public ImGuiWidget {
  GraphicsView(std::shared_ptr<GraphicsScene> scene = nullptr,
               bool enableDebug = false);
  ~GraphicsView();

  void lookAt(ImVec2 p);
  void moveViewTo(ImVec2 p);

  void ImGuiDraw() override;
  virtual void ImGuiLayout() override;

  std::shared_ptr<GraphicsScene> scene_;
  ImVec2 lookAt_;
  bool debug_;
};