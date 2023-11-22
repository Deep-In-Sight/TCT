#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>

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

static inline bool operator==(const ImRect& lhs, const ImRect& rhs) {
  return lhs.Min == rhs.Min && lhs.Max == rhs.Max;
}

static inline bool operator!=(const ImRect& lhs, const ImRect& rhs) {
  return lhs.Min != rhs.Min || lhs.Max != rhs.Max;
}

std::vector<cv::Mat> splitChannels(const cv::Mat& image);

struct ImVec2i {
  ImVec2i() : x(0), y(0) {}
  ImVec2i(int x, int y) : x(x), y(y) {}
  ImVec2i(const ImVec2& v) : x((int)v.x), y((int)v.y) {}

  operator ImVec2() const { return ImVec2((float)x, (float)y); }

  int x, y;
};

struct ImRecti {
  ImRecti() : Min(0, 0), Max(0, 0) {}
  ImRecti(int x0, int y0, int x1, int y1) : Min(x0, y0), Max(x1, y1) {}
  ImRecti(const ImVec2i& min, const ImVec2i& max) : Min(min), Max(max) {}
  ImRecti(const ImRect& r) : Min(r.Min), Max(r.Max) {}

  operator ImRect() const { return ImRect(Min, Max); }
  ImVec2i Min, Max;
};

struct ScrollingBuffer {
  ScrollingBuffer(int max_size = 2000);
  void AddPoint(float x, float y);
  void Erase();
  bool IsEmpty();

  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  int Total;
};

void CreateFontAtlas();

void HelpMarker(const char* desc);