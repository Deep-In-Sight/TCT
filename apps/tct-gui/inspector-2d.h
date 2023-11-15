#pragma once
#include <memory>

#include "graphics-view.h"
#include "imgui-widget.h"

struct InspectorGraphicsView;
struct Inspector2D : public ImGuiWidget {
  Inspector2D();
  ~Inspector2D();

  void ImGuiDraw() override;
  void ImGuiLayout() override;

  void DrawMenu();

  void handleMouse();

  std::shared_ptr<GraphicsScene> scene_;
  std::shared_ptr<InspectorGraphicsView> view_;

  bool firstFrame;
  bool imageSizeChanged;
  bool windowChanged;
};