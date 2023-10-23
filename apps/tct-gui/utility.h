#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <opencv2/core.hpp>

#define IMCOL32_BLACK IM_COL32(0, 0, 0, 255)
#define IMCOL32_WHITE IM_COL32(255, 255, 255, 255)
#define IMCOL32_RED IM_COL32(255, 0, 0, 255)
#define IMCOL32_GREEN IM_COL32(0, 255, 0, 255)
#define IMCOL32_BLUE IM_COL32(0, 0, 255, 255)
#define IMCOL32_YELLOW IM_COL32(255, 255, 0, 255)
#define IMCOL32_CYAN IM_COL32(0, 255, 255, 255)
#define IMCOL32_MAGENTA IM_COL32(255, 0, 255, 255)

void UploadCvMatToGpuTexture(const cv::Mat& image, GLuint* textureId,
                             ImVec2* size = nullptr);

static inline bool operator==(const ImVec2& lhs, const ImVec2& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

static inline bool operator!=(const ImVec2& lhs, const ImVec2& rhs) {
  return lhs.x != rhs.x || lhs.y != rhs.y;
}