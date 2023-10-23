#include "image-widget.h"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// #include "application.h"
#include "utility.h"

void drawRuler(RulerOrientation orientation, ImRect bb, ImVec2 majorMinor,
               float zoom, float originOffset);

ImageWidget::ImageWidget() {
  imageTextureId = 0;
  imageOffset = ImVec2(0.0f, 0.0f);
  showRulers = false;
  rulerSize = ImVec2(40.0f, 25.0f);
  zoomXY = ImVec2(1.0f, 1.0f);
  viewMode = ViewMode::kViewOriginal;
  cv::Mat img = cv::imread("./data/images/MyImage01.jpg");
  setImage(img);
}

ImageWidget::~ImageWidget() {}

/**
 * @brief update the image texture with new image
 *
 * @param image
 */
void ImageWidget::setImage(cv::Mat& image) {
  // convert image to RGBA
  UploadCvMatToGpuTexture(image, &imageTextureId, &imageSize);
}

/**
 * @brief calculate the zoom factor to fit image according to view mode
 *
 * @param mode
 */
void ImageWidget::setViewMode(ViewMode mode) {
  viewMode = mode;
  float zoomX = contentRect.GetWidth() / imageSize.x;
  float zoomY = contentRect.GetHeight() / imageSize.y;

  imageOffset = ImVec2(0.0f, 0.0f);

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

/**
 * @brief add zoomPercent to current zoom level. If anchor is set, the anchor is
 * distance from mouse to top left of content region rect. If anchor is not set,
 * use center of view as anchor.
 *
 * @param zoomPercent
 * @param anchor
 */
void ImageWidget::setZoom(float zoomPercent, ImVec2 anchor) {
  // calculate new size
  ImVec2 currentSize = imageSize * zoomXY;
  ImVec2 nextZoomXY = zoomXY + ImVec2(zoomPercent, zoomPercent);
  if (nextZoomXY.x < 0.1f || nextZoomXY.y < 0.1f) {
    return;
  }
  zoomXY = nextZoomXY;
  ImVec2 newSize = imageSize * zoomXY;

  // if not anchored, zoom from center of image view, otherwise shift anchor
  // from window space to image view
  if (anchor.x < 0 && anchor.y < 0) {
    anchor = mainImageRect.GetSize() / 2.0f;
  } else {
    anchor = anchor - mainImageRect.Min;
  }

  // calculate new offset so that the anchor stay at the same position
  // (anchor - imageOffset) / currentSize = (anchor - newImageOffset) / newSize
  ImVec2 newImageOffset =
      anchor - ((anchor - imageOffset) / currentSize) * newSize;
  imageOffset = newImageOffset;
}

/**
 * @brief enable/disable both the horizontal and vertical rulers
 *
 * @param enable
 */
void ImageWidget::setShowRulers(bool enable) {
  if (enable != showRulers) {
    showRulers = enable;
    // relayout
    ImGuiLayout();
  }
}

/**
 * @brief handle mouse scroll event.
 *
 * @param mousePos in window space
 * @param scroll
 */
void ImageWidget::onMouseScroll(ImVec2 mousePos, float scroll) {
  float step = (scroll > 0) ? 0.1f : -0.1f;
  setZoom(step, mousePos);
}

/**
 * @brief draw the image and rulers
 *
 */
void ImageWidget::ImGuiDraw() {
  DrawMainImage();
  DrawExtraGraphics();
}

void ImageWidget::ImGuiLayout() {
  // setup boundboxes
  if (showRulers) {
    rulerRect = contentRect;
    mainImageRect = rulerRect;
    mainImageRect.Min = mainImageRect.Min + rulerSize;
  } else {
    mainImageRect = contentRect;
  }

  // update viewmode when window reizes
  if (viewMode != ViewMode::kViewOriginal) {
    setViewMode(viewMode);
  }
}

void ImageWidget::DrawMainImage() {
  ImVec2 currentSize = imageSize * zoomXY;
  ImRect fullImageRect;
  fullImageRect.Min = mainImageRect.Min + imageOffset;
  fullImageRect.Max = fullImageRect.Min + currentSize;
  clipRect = fullImageRect;
  clipRect.ClipWithFull(mainImageRect);
  // early return if the image is displayed outside of the mainImageRect area
  if (clipRect.GetArea() <= 0.0f) {
    return;
  }

  ImVec2 uv0 = (clipRect.Min - fullImageRect.Min) / currentSize;
  ImVec2 uv1 = (clipRect.Max - fullImageRect.Min) / currentSize;
  ImVec2 displaySize = clipRect.GetSize();

  ImGui::SetCursorPos(clipRect.Min);
  ImGui::Image((void*)(intptr_t)imageTextureId, displaySize, uv0, uv1);
}

void ImageWidget::DrawExtraGraphics() {
  // the drawList->AddXXX() functions use absolute screen coordinates, so bb is
  // translated from window space to screen space
  if (showRulers) {
    ImRect bb = rulerRect;
    bb.Translate(ImGui::GetWindowPos());

    ImVec2 majorMinor = ImVec2(100.0f, 10.0f);
    ImRect hRulerRect = ImRect(ImVec2(bb.Min.x + rulerSize.x, bb.Min.y),
                               ImVec2(bb.Max.x, bb.Min.y + rulerSize.y));
    ImRect vRulerRect = ImRect(ImVec2(bb.Min.x, bb.Min.y + rulerSize.y),
                               ImVec2(bb.Min.x + rulerSize.x, bb.Max.y));

    drawRuler(RulerOrientation::kRulerHorizontal, hRulerRect, majorMinor,
              zoomXY.x, imageOffset.x);
    drawRuler(RulerOrientation::kRulerVertical, vRulerRect, majorMinor,
              zoomXY.y, imageOffset.y);
  }
}

/**
 * @brief draw ticks and text on the ruler
 *
 * @param horiz true if horizontal ruler, false if vertical ruler
 * @param bb wraps up all the ticks (global screen space)
 * @param zeroOffset offset from ruler edge to tick zero (can be outside of bb)
 * @param spacing distance between ticks
 * @param addText true if text labels are added to the ticks
 * @param increment increment on the label
 */
void drawTicks(RulerOrientation orientation, ImRect bb, float zeroOffset,
               float spacing, bool addText, float textIncrement) {
  auto drawList = ImGui::GetWindowDrawList();
  bool isHoriz = (orientation == RulerOrientation::kRulerHorizontal);
  if (spacing < 1.0f) {
    return;
  }
  float offset = std::fmod(zeroOffset, spacing);
  offset += (offset < 0.0f) ? spacing : 0.0f;
  float firstTickValue = (offset - zeroOffset) / spacing * textIncrement;
  float length = (isHoriz) ? bb.GetWidth() : bb.GetHeight();
  int numTicks = (int)((length - offset) / spacing) + 1;
  char text[16];
  for (int i = 0; i < numTicks; i++) {
    float tickPos = offset + i * spacing;
    ImVec2 tickMin = (isHoriz) ? ImVec2(bb.Min.x + tickPos, bb.Min.y)
                               : ImVec2(bb.Min.x, bb.Min.y + tickPos);
    ImVec2 tickMax =
        (isHoriz) ? ImVec2(tickMin.x, bb.Max.y) : ImVec2(bb.Max.x, tickMin.y);
    drawList->AddLine(tickMin, tickMax, IMCOL32_WHITE);
  }

  if (addText) {
    for (int i = 0; i < numTicks; i++) {
      float tickPos = offset + i * spacing;
      int tickValue = (int)(firstTickValue + i * textIncrement);
      sprintf(text, "%d", tickValue);
      ImVec2 textSize = ImGui::CalcTextSize(text);
      ImVec2 textPos =
          (isHoriz) ? ImVec2(bb.Min.x + tickPos, bb.Min.y - textSize.y - 2.0f)
                    : ImVec2(bb.Min.x - textSize.x - 2.0f, bb.Min.y + tickPos);

      drawList->AddText(textPos, IMCOL32_WHITE, text);
    }
  }
}

/**
 * @brief draw one ruler: background, main line, major/minor ticks, major text
 * labels
 *
 * @param orientation true if horizontal ruler, false if vertical ruler
 * @param bb bounding box that wraps up the ruler (global screen space)
 * @param majorMinor frequency of major and minor ticks
 * @param zoom zoom factor to calculate the spacing between ticks
 * @param zeroOffset offset from edge of bb to the 0 point of the ruler
 */
void drawRuler(RulerOrientation orientation, ImRect bb, ImVec2 majorMinor,
               float zoom, float zeroOffset) {
  bool isHoriz = (orientation == RulerOrientation::kRulerHorizontal);
  float rulerLength = (isHoriz) ? bb.GetWidth() : bb.GetHeight();
  float majorTickSize = 5.0f;
  float minorTickSize = 2.0f;
  ImRect bb2 = bb;
  // a bit separation between ruler and image
  ImVec2 padding = (isHoriz) ? ImVec2(0.0f, 2.0f) : ImVec2(2.0f, 0.0f);
  bb2.Max = bb2.Max - padding;
  ImRect majorTickBb = bb2;
  ImRect minorTickBb = bb2;
  ImRect labelBb = bb2;
  if (isHoriz) {
    majorTickBb.Min.y = majorTickBb.Max.y - majorTickSize;
    minorTickBb.Min.y = minorTickBb.Max.y - minorTickSize;
    labelBb.Max.y = majorTickBb.Min.y - 2.0f;  // draw text above major ticks
  } else {
    majorTickBb.Min.x = majorTickBb.Max.x - majorTickSize;
    minorTickBb.Min.x = minorTickBb.Max.x - minorTickSize;
    labelBb.Max.x = majorTickBb.Min.x - 2.0f;
  }

  auto drawList = ImGui::GetWindowDrawList();
  // draw background
  drawList->AddRectFilled(bb2.Min, bb2.Max, IM_COL32(255, 255, 255, 100));
  // draw the main line
  auto lineStart = (isHoriz) ? bb2.GetBL() : bb2.GetTR();
  auto lineEnd = (isHoriz) ? bb2.GetBR() : bb2.GetBR();
  drawList->AddLine(lineStart, lineEnd, IMCOL32_WHITE);

  auto spacing = majorMinor * zoom;
  // major ticks
  drawTicks(orientation, majorTickBb, zeroOffset, spacing.x, true,
            majorMinor.x);
  // minor ticks
  drawTicks(orientation, minorTickBb, zeroOffset, spacing.y, false, -1.0f);
  // major text labels
  // drawTickLabel(orientation, labelBb, zeroOffset, spacing.x, majorMinor.x);
}