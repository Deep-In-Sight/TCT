#pragma once
#include <GLFW/glfw3.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <opencv2/core.hpp>
#include <vector>

#include "imgui-widget.h"

enum class ViewMode { kViewFit, kViewStretch, kViewOriginal };
enum class RulerOrientation { kRulerHorizontal, kRulerVertical };

struct ImageWidget : public ImGuiWidget {
  ImageWidget();
  ~ImageWidget();

  void setImage(cv::Mat& image);
  void setViewMode(ViewMode mode);
  void setZoom(float zoomPercent, ImVec2 anchor = ImVec2(-1.0f, -1.0f));
  void setShowRulers(bool enable);

  ImVec2 mousePosToImageUV(ImVec2 mousePos);

  void onMouseScroll(ImVec2 mousePos, float scroll);

  void ImGuiDraw() override;
  void ImGuiLayout() override;
  void DrawMainImage();
  void DrawExtraGraphics();

  bool firstFrame;
  GLuint imageTextureId;
  ImVec2 imageSize;
  ImVec2 imageOffset;
  ImVec2 rulerSize;
  ImVec2 zoomXY;
  ViewMode viewMode;
  bool showRulers;

  ImRect mainImageRect;
  ImRect clipRect;
  ImRect rulerRect;
};