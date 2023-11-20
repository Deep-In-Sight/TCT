#pragma once
#include <opencv2/core.hpp>

#include "graphics-item.h"

struct GraphicLineItem : public GraphicsItem {
  GraphicLineItem(ImVec2 p1, ImVec2 p2, std::string name = "");
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
  // make a rectangle and 2 semicircles at both ends wrapping around the line
  // for easy hit test
  int hitTestMargin_ = 1;
};

struct GraphicRectItem : public GraphicsItem {
  GraphicRectItem(ImVec2 p1, ImVec2 p2, std::string name = "");
  void modify(ImVec2 p1, ImVec2 p2);
  void modify(ImRect r);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
};

struct GraphicPolygonItem : public GraphicsItem {
  GraphicPolygonItem(std::vector<ImVec2> points, std::string name = "");
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
};

struct GraphicImageItem : public GraphicsItem {
  GraphicImageItem(std::string name = "");
  GraphicImageItem(cv::Mat& image, std::string name = "");
  void setImage(cv::Mat& image);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
  ImVec2 toOriginalUV(ImVec2 uv);
  ImVec2 imageSize_;
  std::vector<ImVec2> beforeClippedGeometries_;
  ImTextureID imageTextureId_;
  ImVec2 uv0, uv1;
};

struct GraphicTextItem : public GraphicsItem {
  GraphicTextItem(std::string text, ImVec2 pos, std::string name = "");
  GraphicTextItem(std::string name = "");
  void setText(std::string text);
  void setAnchor(int anchor);
  void setBackgroud(bool enable);
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
  bool background_;
  std::string text_;
  int anchor_;
  ImRect bb_;
};

struct Ruler : public GraphicsItem {
  /**
   * @brief Construct a new Ruler graphic item
   *
   * @param horizontal horizontal or vertical
   * @param min
   * @param max
   * @param major
   * @param minor
   * @param size height of horizontal ruler or width of vertical ruler
   * @param name
   * @param parent
   */
  Ruler(bool horizontal, int min = -1000, int max = 1000, int major = 100,
        int minor = 10, float size = 40, std::string name = "");
  ~Ruler();
  void paintBegin() override;
  void paintEnd() override;

  float oldFontSize_;

  bool orientation_;
  void highlight(int value);
  GraphicsItemPtr highlightItem_;
  ImColor backgroundColor_ = ImColor(255, 255, 255, 125);
  ImColor highlightColor_ = ImColor(255, 0, 0, 255);
  ImColor tickColor_ = ImColor(255, 255, 255, 255);
  ImColor textColor_ = ImColor(255, 255, 255, 255);
  ImColor lineColor_ = ImColor(255, 255, 255, 255);
  float majorSize_ = 5.0f;
  float minorSize_ = 2.0f;
  float padding = 4.0f;
};

struct CrossHairItem : public GraphicsItem {
  CrossHairItem(std::string name = "", ImVec2 pos = ImVec2(0, 0));
  void paintSelf() override;
  void clipSelf(ImRect r) override;
  bool hitTest(ImVec2 p) override;
  ImVec2 pos_;
  float sizeDiv2_ = 5.0f;
  ImColor color_ = ImColor(255, 0, 0, 255);
  float thickness_ = 1.0f;
};