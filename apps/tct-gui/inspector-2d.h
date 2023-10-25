#pragma once
#include <memory>

#include "image-widget.h"
#include "imgui-widget.h"

struct Inspector2D : public ImGuiWidget {
  Inspector2D();
  ~Inspector2D();

  void ImGuiDraw() override;
  void ImGuiLayout() override;

  void DrawMenu();

  void onMouseMove(ImVec2 mousePos);
  void onMouseScroll(ImVec2 mousePos, float scroll);

  std::shared_ptr<ImageWidget> imageWidget;
  std::shared_ptr<ImageWidget> imageWidget2;

  bool firstFrame;
  bool imageSizeChanged;
  bool windowSizeChanged;
  ViewMode viewMode;
};