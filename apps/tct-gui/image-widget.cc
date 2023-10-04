#include "image-widget.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "application.h"
#include "utility.h"

ImageWidget::ImageWidget() {
  imageTextureId = 0;
  offset = ImVec2(0.0f, 0.0f);
  zoomXY = ImVec2(1.0f, 1.0f);
  viewMode = ViewMode::kViewOriginal;
  cv::Mat img = cv::imread("./data/images/MyImage01.jpg");
  setImage(img);
}

ImageWidget::~ImageWidget() {}

void ImageWidget::setImage(cv::Mat& image) {
  // convert image to RGBA
  UploadCvMatToGpuTexture(image, &imageTextureId, &imageSize);
}

void ImageWidget::ImGuiDraw() {
  auto io = ImGui::GetIO();

  ImVec2 currentSize = imageSize * zoomXY;

  // image is display at imageOffset from top left of content region rect
  ImGui::SetCursorPos(offset + contentRect.Min);
  ImGui::Image((void*)(intptr_t)imageTextureId, currentSize);
}

void ImageWidget::ImGuiLayout() {}

void ImageWidget::setViewMode(ViewMode mode) {
  viewMode = mode;
  float zoomX = contentRect.GetWidth() / imageSize.x;
  float zoomY = contentRect.GetHeight() / imageSize.y;

  offset = ImVec2(0.0f, 0.0f);

  switch (viewMode) {
    case ViewMode::kViewFit: {
      float zoom = (zoomX < zoomY) ? zoomX : zoomY;
      zoomXY = ImVec2(zoom, zoom);
      break;
    }
    case ViewMode::kViewStretch: {
      zoomXY = ImVec2(zoomX, zoomY);
      break;
    }
    case ViewMode::kViewOriginal: {
      zoomXY = ImVec2(1.0f, 1.0f);
      break;
    }
    default: {
      zoomXY = ImVec2(1.0f, 1.0f);
    }
  }
}

void ImageWidget::setZoom(float zoom, ImVec2 anchor) {
  // calculate new size
  ImVec2 currentSize = imageSize * zoomXY;
  zoomXY = zoomXY + ImVec2(zoom, zoom);
  ImVec2 newSize = imageSize * zoomXY;

  // translate position in window to position in position in contentRect
  // if anchor is not set, use center of image
  ImVec2 anchorOffset;
  if (anchor.x > 0 && anchor.y > 0) {
    anchorOffset = anchor - contentRect.Min;
  } else {
    anchorOffset = offset + currentSize / 2;
  }

  // calculate new offset so that the anchor stay at the same position
  // (anchorOffset - offset) / currentSize = (anchorOffset - newOffset) /
  // newSize
  ImVec2 newOffset =
      anchorOffset - ((anchorOffset - offset) / currentSize) * newSize;
  offset = newOffset;
}