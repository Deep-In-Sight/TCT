#pragma once
#include <opencv2/core.hpp>

#include "graphics-item.h"

struct GraphicLineItem : public GraphicsItem {
  GraphicLineItem(ImVec2 p1, ImVec2 p2, std::string name = "",
                  GraphicsItem* parent = nullptr);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
  // make a rectangle and 2 semicircles at both ends wrapping around the line
  // for easy hit test
  int hitTestMargin_ = 1;
};

struct GraphicRectItem : public GraphicsItem {
  GraphicRectItem(ImVec2 p1, ImVec2 p2, std::string name = "",
                  GraphicsItem* parent = nullptr);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
};

struct GraphicPolygonItem : public GraphicsItem {
  GraphicPolygonItem(std::vector<ImVec2> points, std::string name = "",
                     GraphicsItem* parent = nullptr);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
};

enum class FitMode { kFill, kStretch, kOriginal };

struct GraphicImageItem : public GraphicsItem {
  GraphicImageItem(std::string name = "", GraphicsItem* parent = nullptr);
  GraphicImageItem(cv::Mat& image, std::string name = "",
                   GraphicsItem* parent = nullptr);
  void setImage(cv::Mat& image);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
  void setFitMode(FitMode mode);
  ImVec2 imageSize_;
  ImTextureID imageTextureId_;
  ImVec2 uv0, uv1;
};

struct GraphicTextItem : public GraphicsItem {
  GraphicTextItem(std::string text, std::string name = "",
                  GraphicsItem* parent = nullptr);
  GraphicTextItem(std::string name = "", GraphicsItem* parent = nullptr);
  void setText(std::string text);
  void setCorner(int corner);
  void setBackgroud(bool enable);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
  bool background_;
  std::string text_;
  int corner_;
  ImRect lastRect_;
};