#include "inspector-2d.h"

#include <iostream>
#include <opencv2/imgcodecs.hpp>

#include "graphics-item-impl.h"
#include "graphics-layout.h"
#include "utility.h"

/*
 * ┌────────hLayoutTop─────────────────────────────────────────────────────┐
 * │ ┌────vLayout0────────────────────┐ ┌────vLayout1────────────────────┐ │
 * │ │ ┌─────hLayout00──────────────┐ │ │ ┌─────hLayout01──────────────┐ │ │
 * │ │ │ ┌─nothing─┐ ┌──hRuler0───┐ │ │ │ │ ┌─nothing─┐ ┌───hRuler1──┐ │ │ │
 * │ │ │ │         │ │            │ │ │ │ │ │         │ │            │ │ │ │
 * │ │ │ │         │ │            │ │ │ │ │ │         │ │            │ │ │ │
 * │ │ │ │         │ │            │ │ │ │ │ │         │ │            │ │ │ │
 * │ │ │ └─────────┘ └────────────┘ │ │ │ │ └─────────┘ └────────────┘ │ │ │
 * │ │ └────────────────────────────┘ │ │ └────────────────────────────┘ │ │
 * │ │ ┌─────hLayout10──────────────┐ │ │ ┌─────hLayout11──────────────┐ │ │
 * │ │ │ ┌─vRuler0─┐ ┌──image0────┐ │ │ │ │ ┌─vRuler1─┐ ┌────image1──┐ │ │ │
 * │ │ │ │         │ │            │ │ │ │ │ │         │ │            │ │ │ │
 * │ │ │ │         │ │            │ │ │ │ │ │         │ │            │ │ │ │
 * │ │ │ │         │ │            │ │ │ │ │ │         │ │            │ │ │ │
 * │ │ │ └─────────┘ └────────────┘ │ │ │ │ └─────────┘ └────────────┘ │ │ │
 * │ │ └────────────────────────────┘ │ │ └────────────────────────────┘ │ │
 * │ └────────────────────────────────┘ └────────────────────────────────┘ │
 * └───────────────────────────────────────────────────────────────────────┘
 *
 * by asciiflow.com
 */
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
  std::shared_ptr<GraphicTextItem> mouseLabelItem;
};

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
    imageItems_[i]->scale(scale.x, scale.y);
  }
}

void InspectorGraphicsView::setImage(cv::Mat& image) {
  imageItems_[0]->setImage(image);
  imageItems_[1]->setImage(image);
}

Inspector2D::Inspector2D() {
  scene_ = std::make_shared<GraphicsScene>();
  view_ = std::make_shared<InspectorGraphicsView>(scene_);
  auto image = cv::imread("./data/images/MyImage01.jpg");
  view_->setImage(image);

  firstFrame = true;
  imageSizeChanged = false;
  windowChanged = false;
}

Inspector2D::~Inspector2D() {}

void Inspector2D::ImGuiDraw() {
  auto io = ImGui::GetIO();
  static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar |
                                        ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoScrollWithMouse;

  bool needRelayout =
      firstFrame || imageSizeChanged || windowChanged || view_->layoutChanged_;
  static ImVec2 lastWindowPos = ImVec2(0.0f, 0.0f);
  static ImVec2 lastWindowSize = ImVec2(0.0f, 0.0f);
  static ImVec2 lastMousePos = ImVec2(0.0f, 0.0f);

  if (ImGui::Begin("Inspector 2D", nullptr, windowFlags)) {
    if (needRelayout) {
      ImGuiLayout();
    }

    // draw menus
    DrawMenu();

    // child widgets draw
    view_->ImGuiDraw();

    // handle events
    if (ImGui::IsWindowHovered()) {
      handleMouse();
    }

    if (firstFrame) {
      firstFrame = false;
    }

    if (ImGui::GetWindowSize() != lastWindowSize ||
        ImGui::GetWindowPos() != lastWindowPos) {
      windowChanged = true;
      lastWindowPos = ImGui::GetWindowPos();
      lastWindowSize = ImGui::GetWindowSize();
    } else {
      windowChanged = false;
    }
  }
  ImGui::End();
}

void Inspector2D::ImGuiLayout() {
  contentRect = ImRect(ImGui::GetWindowContentRegionMin(),
                       ImGui::GetWindowContentRegionMax());
  view_->contentRect = contentRect;
  view_->ImGuiLayout();
}

void Inspector2D::DrawMenu() {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("View")) {
      if (ImGui::BeginMenu("View Mode")) {
        static bool menuOriginalSelected = true;
        static bool menuFitSelected = false;
        static bool menuStretchSelected = false;
        ViewMode lastvm = view_->viewMode_;
        ViewMode vm = view_->viewMode_;
        if (ImGui::MenuItem("Original", nullptr, &menuOriginalSelected)) {
          vm = ViewMode::kViewOriginal;
        }
        if (ImGui::MenuItem("Fit", nullptr, &menuFitSelected)) {
          vm = ViewMode::kViewFit;
        }
        if (ImGui::MenuItem("Stretch", nullptr, &menuStretchSelected)) {
          vm = ViewMode::kViewStretch;
        }
        if (lastvm != vm) {
          view_->setImageFitMode(vm);
        }
        menuOriginalSelected = (vm == ViewMode::kViewOriginal);
        menuFitSelected = (vm == ViewMode::kViewFit);
        menuStretchSelected = (vm == ViewMode::kViewStretch);
        ImGui::EndMenu();  // end menu View / View Mode
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Zoom In", nullptr, nullptr)) {
        // imageWidget->setZoom(0.1f);
      }
      if (ImGui::MenuItem("Zoom Out", nullptr, nullptr)) {
        // imageWidget->setZoom(-0.1f);
      }
      ImGui::Separator();
      bool rulersEnabled = view_->rulersEnabled_;
      if (ImGui::MenuItem("Rulers", nullptr, &rulersEnabled)) {
        view_->enableRulers(rulersEnabled);
      }
      bool sideBySide = view_->sideBySide_;
      if (ImGui::MenuItem("Side By Side", nullptr, &sideBySide)) {
        view_->enableSideBySide(sideBySide);
      }

      ImGui::EndMenu();  // end menu View
    }
    ImGui::EndMenuBar();
  }
}

void Inspector2D::handleMouse() {
  auto io = ImGui::GetIO();

  if (io.WantCaptureMouse) {
    auto mousePos = io.MousePos;
    if (ImGui::IsMouseClicked(0)) {
      auto item = view_->scene_->itemAt(mousePos);
      if (item) {
        std::cout << item->name_ << " clicked" << std::endl;
      }
    }
    if (ImGui::IsMouseDragging(0)) {
      auto item = view_->scene_->itemAt(mousePos);
      if (item) {
        std::cout << item->name_ << " dragged" << std::endl;
        std::cout << "current pos: " << mousePos.x << "," << mousePos.y
                  << "delta: " << io.MouseDelta.x << "," << io.MouseDelta.y
                  << std::endl;
      }
    }
  }
}