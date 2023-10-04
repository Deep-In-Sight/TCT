#pragma once
#include <memory>

#include "image-widget.h"
#include "imgui-widget.h"

struct Inspector2D : public ImGuiWidget {
  Inspector2D();
  ~Inspector2D();

  void ImGuiDraw() override;
  void ImGuiLayout() override;

  std::shared_ptr<ImageWidget> imageWidget;

  bool firstFrame;
  ViewMode viewMode;
};