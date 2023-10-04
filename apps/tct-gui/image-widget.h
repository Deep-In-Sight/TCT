#pragma once
#include <GLFW/glfw3.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <opencv2/core.hpp>
#include <vector>

#include "imgui-widget.h"

enum class ViewMode { kViewFit, kViewStretch, kViewOriginal };

struct ImageWidget : public ImGuiWidget {
  ImageWidget();
  ~ImageWidget();

  void setImage(cv::Mat& image);
  void setViewMode(ViewMode mode);
  void setZoom(float zoom, ImVec2 anchor = ImVec2(-1.0f, -1.0f));

  void ImGuiDraw() override;
  void ImGuiLayout() override;

  bool firstFrame;
  GLuint imageTextureId;
  ImVec2 imageSize;
  ImVec2 offset;
  ImVec2 zoomXY;
  ViewMode viewMode;
};