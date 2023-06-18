#include "ImageInspectorWidget.hpp"

#include <QGraphicsView>

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

  pixmapItem_ = nullptr;
}

void ImageInspectorGraphicsScene::contextMenuEvent(
    QGraphicsSceneContextMenuEvent* event) {
  QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
  lastPos_ = event->scenePos();

  if (item) {
    QMenu* menu;
    if (dynamic_cast<QGraphicsPixmapItem*>(item)) {
      menu = menuAddInspector_;
    } else {
      menu = menuDeleteInspector_;
    }

    menu->exec(event->screenPos());
  }

  event->accept();
}

void ImageInspectorGraphicsScene::SetImage(const QPixmap& pixmap) {
  cout << "ImageInspectorGraphicsScene::SetImage" << endl;
  if (pixmapItem_ == nullptr) {
    pixmapItem_ = addPixmap(pixmap);
    setSceneRect(pixmap.rect());
  } else {
    pixmapItem_->setPixmap(pixmap);
  }
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

VideoSinkViewer::VideoSinkViewer(QWidget* parent)
    : QWidget(parent), BaseSink() {
  layout_ = new QVBoxLayout(this);
  scene_ = new ImageInspectorGraphicsScene(this);
  view_ = new QGraphicsView(scene_, this);

  layout_->addWidget(view_);

  connect(this, &VideoSinkViewer::newPixMap, this,
          &VideoSinkViewer::onNewPixMap);

  setWindowFlags(Qt::WindowStaysOnTopHint);
}

VideoSinkViewer::~VideoSinkViewer() {}

void VideoSinkViewer::SinkFrame(Mat& frame) {
  int height = frame.size[1];
  int width = frame.size[2];
  uint8_t* data = frame.data;
  if (channel_ == ViewerChannel::Amplitude) {
    data = frame.data + width * height * frame.elemSize();
  }

  Mat depth(height, width, CV_32FC1, data);
  Mat depth_norm;
  cv::normalize(depth, depth_norm, 0, 255, cv::NORM_MINMAX, CV_8UC1);
  cv::cvtColor(depth_norm, depth_norm, cv::COLOR_GRAY2RGB);
  cv::imwrite("/home/linh/depth.png", depth_norm);
  QPixmap pixmap = QPixmap::fromImage(QImage(depth_norm.data, depth_norm.cols,
                                             depth_norm.rows, depth_norm.step,
                                             QImage::Format_RGB888));

  emit newPixMap(pixmap);
}

void VideoSinkViewer::onNewPixMap(const QPixmap& pixmap) {
  scene_->SetImage(pixmap);
  this->setMinimumSize(pixmap.size());
  view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void VideoSinkViewer::SetChannel(ViewerChannel channel) { channel_ = channel; }