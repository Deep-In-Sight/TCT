#include "ImageInspectorGraphicsScene.hpp"

ImageInspectorGraphicsScene::ImageInspectorGraphicsScene(QWidget* parent)
    : QGraphicsScene(parent) {
  // Create the context menu
  menuAddInspector_ = new QMenu("Add Inspector");
  QAction* addRowAct = menuAddInspector_->addAction("Add Row Scanner");
  QAction* addColAct = menuAddInspector_->addAction("Add Column Scanner");
  QAction* addHistAct = menuAddInspector_->addAction("Add Histogram ROI");
  QAction* addTrackerAct = menuAddInspector_->addAction("Add Tracker Point");

  connect(addRowAct, &QAction::triggered, this,
          &ImageInspectorGraphicsScene::addRowScanner);
  connect(addColAct, &QAction::triggered, this,
          &ImageInspectorGraphicsScene::addColScanner);
  connect(addHistAct, &QAction::triggered, this,
          &ImageInspectorGraphicsScene::addHistROI);
  connect(addTrackerAct, &QAction::triggered, this,
          &ImageInspectorGraphicsScene::addTrackerPoint);

  menuDeleteInspector_ = new QMenu("Delete Inspector");
  QAction* delInspector = menuDeleteInspector_->addAction("Delete Inspector");
  connect(delInspector, &QAction::triggered, this,
          &ImageInspectorGraphicsScene::deleteChildInspector);
}

void ImageInspectorGraphicsScene::contextMenuEvent(
    QGraphicsSceneContextMenuEvent* event) override {
  QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
  lastPos_ = event->scenePos();

  if (item) {
    QMenu* menu;
    if (dynamic_cast<QGraphicsPixmapItem*>(item)) {
      menu = menuAddInspector_;
    } else {
      menu = menuDeleteInspector_;
    }

    menu.exec(event->screenPos());
  }

  event->accept();
}

void ImageInspectorGraphicsScene::deleteChildInspector() {
  // Delete the selected item
  removeItem(selectedItem_);
}

void ImageInspectorGraphicsScene::addRowScanner() {
  cout << "addRowScanner triggered" << endl;
}
void ImageInspectorGraphicsScene::addColScanner() {
  cout << "addColScanner triggered" << endl;
}
void ImageInspectorGraphicsScene::addHistROI() {
  cout << "addHistROI triggered" << endl;
}
void ImageInspectorGraphicsScene::addTrackerPoint() {
  cout << "addTrackerPoint triggered" << endl;
}

void ImageInspectorGraphicsScene::SinkFrame(Mat& frame) {
  // Convert the frame to a pixmap
  // QPixmap pixmap = QPixmap::fromImage(
  //     QImage(frame.data, frame.cols, frame.rows, frame.step,
  //     QImage::Format_RGB888).rgbSwapped());

  // // Create a pixmap item
  // QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(pixmap);

  // // Add the pixmap item to the scene
  // addItem(pixmapItem);

  // // Set the selected item to the pixmap item
  // selectedItem_ = pixmapItem;
}