#pragma once
#include <memory>

#include "graphics-item-impl.h"
#include "graphics-layout.h"
#include "graphics-view.h"

enum class ViewMode { kViewFit, kViewStretch, kViewOriginal };

struct InspectorGraphicsView : public GraphicsView {
  InspectorGraphicsView(std::shared_ptr<GraphicsScene>& scene,
                        bool debug = false);

  void enableSideBySide(bool enable);
  void enableRulers(bool enable);
  void setImageFitMode(ViewMode mode);
  void setImage(cv::Mat& image);
  std::shared_ptr<GraphicLineItem> addLineToImage(int channel, ImVec2 p1,
                                                  ImVec2 p2,
                                                  const std::string& name);
  std::shared_ptr<GraphicRectItem> addRectToImage(int channel, ImVec2 p1,
                                                  ImVec2 p2,
                                                  const std::string& name);
  void buildScene();
  virtual void ImGuiLayout() override;

  void onMouseMove(ImVec2 mousePos);
  void onMouseScroll(ImVec2 mousePos, float scroll);

  // zoom image and rulers, origin is in imageItem coordinate
  void _zoomImage(ImVec2 zoomXY, ImVec2 origin, int leftRight, bool reset);

  bool sideBySide_;
  bool rulersEnabled_;
  ViewMode viewMode_;
  ImRect lastContentRect_;
  bool layoutChanged_;
  float hRulerHeight_ = 25;
  float vRulerWidth_ = 40;
  std::shared_ptr<GraphicsHBoxLayout> hLayoutTop_;
  std::shared_ptr<GraphicsGridLayout> grids_[2];
  std::shared_ptr<GraphicImageItem> imageItems_[2];
  std::shared_ptr<Ruler> hRulerItems_[2];
  std::shared_ptr<Ruler> vRulerItems_[2];
  std::shared_ptr<GraphicTextItem> mouseLabelItem_;
};