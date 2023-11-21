#include "inspector-graphics-view.h"

#include "inspector-2d.h"
#include "utility.h"
/*
 * ┌────────hLayoutTop─────────────────────────────────────────────────────┐
 * │ ┌────vLayout0────────────────────┐ ┌────vLayout1────────────────────┐ │
 * │ │ ┌─────hLayout00──────────────┐ │ │ ┌─────hLayout01──────────────┐ │ │
 * │ │ │ ┌─nothing─┐ ┌──hRuler0───┐ │ │ │ │ ┌─nothing─┐ ┌───hRuler1──┐ │ │ │
 * │ │ │ └─────────┘ └────────────┘ │ │ │ │ └─────────┘ └────────────┘ │ │ │
 * │ │ └────────────────────────────┘ │ │ └────────────────────────────┘ │ │
 * │ │ ┌─────hLayout10──────────────┐ │ │ ┌─────hLayout11──────────────┐ │ │
 * │ │ │ ┌─vRuler0─┐ ┌──image0────┐ │ │ │ │ ┌─vRuler1─┐ ┌────image1──┐ │ │ │
 * │ │ │ │         │ │            │ │ │ │ │ │         │ │            │ │ │ │
 * │ │ │ │         │ │            │ │ │ │ │ │         │ │            │ │ │ │
 * │ │ │ └─────────┘ └────────────┘ │ │ │ │ └─────────┘ └────────────┘ │ │ │
 * │ │ └────────────────────────────┘ │ │ └────────────────────────────┘ │ │
 * │ └────────────────────────────────┘ └────────────────────────────────┘ │
 * └───────────────────────────────────────────────────────────────────────┘
 *
 * by asciiflow.com
 */

InspectorGraphicsView::InspectorGraphicsView(
    std::shared_ptr<GraphicsScene>& scene, bool debug)
    : GraphicsView(scene, debug) {
  buildScene();
  enableSideBySide(true);
  enableRulers(true);
  setImageFitMode(ViewMode::kViewFit);
  layoutChanged_ = true;
}

void InspectorGraphicsView::buildScene() {
  hLayoutTop_ = std::make_shared<GraphicsHBoxLayout>("hlayout_");
  scene_->addItem(hLayoutTop_);
  for (int i = 0; i < 2; i++) {
    auto grid =
        std::make_shared<GraphicsGridLayout>(2, 2, "grid_" + std::to_string(i));
    grid->margins_ = ImVec4(0, 0, 2, 0);
    hLayoutTop_->addChild(grid);

    grid->addChild(std::make_shared<GraphicsLayout>("nothing"));
    grid->addChild(std::make_shared<GraphicsLayout>("hRulerBox"));
    grid->addChild(std::make_shared<GraphicsLayout>("vRulerBox"));
    grid->addChild(std::make_shared<GraphicsLayout>("imageBox"));

    auto hRuler =
        std::make_shared<Ruler>(true, -1000, 1000, 100, 10, hRulerHeight_,
                                "hRuler_" + std::to_string(i));
    (*grid)(0, 1)->addChild(hRuler);
    auto vRuler =
        std::make_shared<Ruler>(false, -1000, 1000, 100, 10, vRulerWidth_,
                                "vRuler_" + std::to_string(i));
    (*grid)(1, 0)->addChild(vRuler);
    auto img = std::make_shared<GraphicImageItem>("image_" + std::to_string(i));
    (*grid)(1, 1)->addChild(img);

    grids_[i] = grid;
    imageItems_[i] = img;
    hRulerItems_[i] = hRuler;
    vRulerItems_[i] = vRuler;
  }

  mouseLabelItem_ = std::make_shared<GraphicTextItem>("", ImVec2(0, 0));
  mouseLabelItem_->background_ = true;
  // white text on black background
  mouseLabelItem_->fillColor_ = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
  mouseLabelItem_->lineColor_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  // avoid returning itself during hittest
  mouseLabelItem_->isClickable_ = false;
  mouseLabelItem_->setAnchor(6);
  hLayoutTop_->addChild(mouseLabelItem_);
}

void InspectorGraphicsView::enableSideBySide(bool enable) {
  sideBySide_ = enable;
  grids_[1]->isHidden_ = !enable;
  layoutChanged_ = true;
}

void InspectorGraphicsView::enableRulers(bool enable) {
  rulersEnabled_ = enable;
  for (int i = 0; i < 2; i++) {
    (*grids_[i])(0, 1)->isHidden_ = !enable;  // hrulerbox
    (*grids_[i])(1, 0)->isHidden_ = !enable;  // vrulerbox
  }
  layoutChanged_ = true;
}

void InspectorGraphicsView::ImGuiLayout() {
  // if (!layoutChanged_) return;

  ImRect viewRect = contentRect;         // location of view inside window
  ImVec2 viewSize = viewRect.GetSize();  // size of view

  // put the toplevel view at the contentRect area
  hLayoutTop_->setPos(ImGui::GetWindowPos() + viewRect.Min);
  hLayoutTop_->clip(ImRect(ImVec2(0, 0), viewSize));

  // now calculate the size of the children
  float w = viewSize.x;
  float h = viewSize.y;
  float gridWidth = (sideBySide_) ? w / 2 : w;
  float gridHeight = h;
  ImVec2 hRulerSize = (rulersEnabled_)
                          ? ImVec2(gridWidth - vRulerWidth_, hRulerHeight_)
                          : ImVec2(0, 0);
  ImVec2 vRulerSize = (rulersEnabled_)
                          ? ImVec2(vRulerWidth_, gridHeight - hRulerHeight_)
                          : ImVec2(0, 0);
  ImVec2 imageSize = (rulersEnabled_) ? ImVec2(hRulerSize.x, vRulerSize.y)
                                      : ImVec2(gridWidth, gridHeight);

  for (int i = 0; i < 2; i++) {
    (*grids_[i])(0, 0)->preferredSize_ = ImVec2(vRulerSize.x, hRulerSize.y);
    (*grids_[i])(0, 1)->preferredSize_ = hRulerSize;
    (*grids_[i])(1, 0)->preferredSize_ = vRulerSize;
    (*grids_[i])(1, 1)->preferredSize_ = imageSize;
  }

  // enjoy
  hLayoutTop_->layout();
  // std::cout << hLayoutTop_->toString();
  setImageFitMode(viewMode_);

  layoutChanged_ = false;
}

void InspectorGraphicsView::setImageFitMode(ViewMode mode) {
  viewMode_ = mode;
  auto imageBox = (*grids_[0])(1, 1);
  ImVec2 imageViewSize = imageBox->preferredSize_;
  if (imageViewSize == ImVec2(0, 0)) return;  // not ready yet

  ImVec2 zoomStretch = imageViewSize / imageItems_[0]->imageSize_;
  float zoomMin = ImMin(zoomStretch.x, zoomStretch.y);
  ImVec2 zoomFit(zoomMin, zoomMin);

  ImVec2 scale = (mode == ViewMode::kViewFit)       ? zoomFit
                 : (mode == ViewMode::kViewStretch) ? zoomStretch
                                                    : ImVec2(1.0f, 1.0f);
  for (int i = 0; i < 2; i++) {
    _zoomImage(scale, ImVec2(0, 0), i, true);
  }
}

void InspectorGraphicsView::setImages(std::vector<cv::Mat> images) {
  imageItems_[0]->setImage(images[0]);
  if (sideBySide_) {
    if (images.size() > 1) {
      imageItems_[1]->setImage(images[1]);
    } else {
      // TODO: write some text saying no data on the right side
      enableSideBySide(false);
    }
  }
}

void InspectorGraphicsView::onMouseMove(ImVec2 mousePos) {
  bool hit0 = imageItems_[0]->hitTest(mousePos);
  bool hit1 = imageItems_[1]->hitTest(mousePos);
  if (!hit0 && !hit1) {
    mouseLabelItem_->isHidden_ = true;
    return;
  }
  int index = (hit0) ? 0 : 1;
  auto item = imageItems_[index];

  mouseLabelItem_->isHidden_ = false;
  ImVec2 mousePosImage = item->mapFromScene(mousePos);
  ImVec2 uv = ImFloor(item->toOriginalUV(mousePos));

  auto values = inspector_->GetPixel(uv.x, uv.y);

  std::ostringstream label;
  label << "Loc: [" << (int)uv.x << "," << (int)uv.y << "]" << std::endl
        << "Depth: " << values[0] << std::endl
        << "Intensity: " << values[1] << std::endl;
  mouseLabelItem_->setText(label.str());
  mouseLabelItem_->setPos(mouseLabelItem_->parent_->mapFromScene(mousePos));

  auto hRuler = hRulerItems_[index];
  auto vRuler = vRulerItems_[index];
  hRuler->highlight(uv.x);
  vRuler->highlight(uv.y);
}

void InspectorGraphicsView::onMouseScroll(ImVec2 mousePos, float scroll) {
  // we zoom when mouse is hovered on the image "area", meaning both image and
  // the markers on it. If we use itemAt to check if mouse is hitting only the
  // image then we have to disable isClickable_ on the markers, which is not
  // ideal, because we may want to do something with the markers using mouse
  // later
  bool hit0 = imageItems_[0]->hitTest(mousePos);
  bool hit1 = imageItems_[1]->hitTest(mousePos);
  if (!hit0 && !hit1) {
    return;
  }

  int index = (hit0) ? 0 : 1;
  auto item = imageItems_[index];
  float zoom = ImPow(10, scroll / 100.0f);
  auto origin = item->mapFromScene(mousePos);

  _zoomImage(ImVec2(zoom, zoom), origin, index, false);
}

void InspectorGraphicsView::_zoomImage(ImVec2 zoomXY, ImVec2 origin,
                                       int leftRight, bool reset) {
  auto imageItem = imageItems_[leftRight];
  auto hRulerItem = hRulerItems_[leftRight];
  auto vRulerItem = vRulerItems_[leftRight];

  ImVec2 imageOrigin = origin;
  ImVec2 hRulerOrigin(origin.x, 0);
  ImVec2 vRulerOrigin(0, origin.y);

  imageItem->setOrigin(imageOrigin);
  imageItem->scale(zoomXY.x, zoomXY.y, !reset);

  hRulerItem->setOrigin(hRulerOrigin);
  hRulerItem->scale(zoomXY.x, 1.0f, !reset);

  vRulerItem->setOrigin(vRulerOrigin);
  vRulerItem->scale(1.0f, zoomXY.y, !reset);
}