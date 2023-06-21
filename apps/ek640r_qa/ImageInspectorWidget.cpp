#include "ImageInspectorWidget.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <QGraphicsView>
#include <opencv2/opencv.hpp>

using namespace spdlog;

static logger* logger_ = stdout_color_mt("ImageInspectorGraphicsScene").get();
static logger* distance_logger_ =
    basic_logger_mt("distanceLogger", "distance.log").get();

ImageInspectorGraphicsScene::ImageInspectorGraphicsScene(QWidget* parent)
    : QGraphicsScene(parent) {
  distance_logger_->set_pattern("%v");
  // Create the context menu
  menuInspector_ = new QMenu("Add Inspector");
  // QAction* addRowAct = menuInspector_->addAction("Add Row Scanner");
  // QAction* addColAct = menuInspector_->addAction("Add Column Scanner");
  // QAction* addHistAct = menuInspector_->addAction("Add Histogram ROI");
  QAction* addTrackerAct = menuInspector_->addAction("Add Tracker Point");
  QAction* clearTrackerAct = menuInspector_->addAction("Clear all markers");
  QAction* saveFrameAct = menuInspector_->addAction("Save Frame");

  // connect(addRowAct, &QAction::triggered, this,
  //         [this]() { addRowScanner(lastPos_.y()); });
  // connect(addColAct, &QAction::triggered, this,
  //         [this]() { addColScanner(lastPos_.x()); });
  /** disable for now because it needs ROI selection not just context menu click
   position */
  // connect(addHistAct, &QAction::triggered, this,
  //         &ImageInspectorGraphicsScene::addHistROI);
  connect(addTrackerAct, &QAction::triggered, this,
          [this]() { addTrackerPoint(lastPos_.toPoint()); });
  connect(clearTrackerAct, &QAction::triggered, this,
          [this]() { deleteAllMarkers(); });
  connect(saveFrameAct, &QAction::triggered, this, [this]() { saveFrame(); });

  menuDeleteInspector_ = new QMenu("Delete Inspector");
  QAction* delInspector = menuDeleteInspector_->addAction("Delete Inspector");
  connect(delInspector, &QAction::triggered, this,
          [this]() { deleteChildInspector(selectedItem_); });

  pixmapItem_ = nullptr;
  frameRateItem_ = nullptr;
}

void ImageInspectorGraphicsScene::contextMenuEvent(
    QGraphicsSceneContextMenuEvent* event) {
  QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
  lastPos_ = event->scenePos();
  selectedItem_ = item;

  if (item) {
    QMenu* menu;
    if (dynamic_cast<QGraphicsPixmapItem*>(item)) {
      menu = menuInspector_;
    } else {
      menu = menuDeleteInspector_;
    }

    menu->exec(event->screenPos());
  }

  event->accept();
}

void ImageInspectorGraphicsScene::SetImage(const QPixmap& pixmap) {
  logger_->info("SetImage: pixmap size: {}x{}", pixmap.width(),
                pixmap.height());
  auto duration = chrono::duration_cast<chrono::milliseconds>(
                      chrono::system_clock::now() - lastFrameTime_)
                      .count();
  float frameRate = 1000.0 / duration;
  addFrameRate(frameRate * 4.5);
  lastFrameTime_ = chrono::system_clock::now();
  if (pixmapItem_ == nullptr) {
    pixmapItem_ = addPixmap(pixmap);
    setSceneRect(pixmap.rect());
  } else {
    pixmapItem_->setPixmap(pixmap);
  }
}

void ImageInspectorGraphicsScene::addCrossHair(QPoint point, float depthVal_) {
  QGraphicsItemGroup* group = new QGraphicsItemGroup();

  QGraphicsLineItem* line1 = new QGraphicsLineItem();
  line1->setLine(point.x() - 6, point.y(), point.x() + 6, point.y());
  line1->setPen(QPen(Qt::red, 1));
  group->addToGroup(line1);

  QGraphicsLineItem* line2 = new QGraphicsLineItem();
  line2->setLine(point.x(), point.y() - 6, point.x(), point.y() + 6);
  line2->setPen(QPen(Qt::red, 1));
  group->addToGroup(line2);

  QGraphicsEllipseItem* circle = new QGraphicsEllipseItem();
  circle->setRect(point.x() - 3, point.y() - 3, 6, 6);
  circle->setPen(QPen(Qt::red, 2));
  group->addToGroup(circle);

  QGraphicsTextItem* text = new QGraphicsTextItem();
  bool down = true;
  bool right = true;
  int pos_x, pos_y;
  text->setPlainText(
      QString("(%1, %2) %3m").arg(point.x()).arg(point.y()).arg(depthVal_));
  if (point.x() + 5 + text->boundingRect().width() > sceneRect().width()) {
    right = false;
  }
  if (point.y() + 5 + text->boundingRect().height() > sceneRect().height()) {
    down = false;
  }
  if (right) {
    pos_x = point.x() + 5;
  } else {
    pos_x = point.x() - 5 - text->boundingRect().width();
  }
  if (down) {
    pos_y = point.y() + 5;
  } else {
    pos_y = point.y() - 5 - text->boundingRect().height();
  }
  text->setPos(pos_x, pos_y);
  text->setDefaultTextColor(Qt::red);

  QGraphicsRectItem* rect = new QGraphicsRectItem(text->boundingRect());
  rect->setBrush(QBrush(Qt::white));
  rect->setPos(pos_x, pos_y);
  group->addToGroup(rect);
  group->addToGroup(text);

  addItem(group);
  markersList.push_back(group);
}

void ImageInspectorGraphicsScene::addFrameRate(float frameRate) {
  if (frameRateItem_ != nullptr) {
    removeItem(frameRateItem_);
  }

  frameRateItem_ = new QGraphicsItemGroup();
  auto frameRateTextItem_ =
      new QGraphicsTextItem(QString("%1 fps").arg(frameRate));
  frameRateTextItem_->setDefaultTextColor(Qt::red);
  auto frameRateRectItem_ =
      new QGraphicsRectItem(frameRateTextItem_->boundingRect());
  frameRateRectItem_->setBrush(QBrush(Qt::white));

  frameRateItem_->addToGroup(frameRateRectItem_);
  frameRateItem_->addToGroup(frameRateTextItem_);
  addItem(frameRateItem_);
  int posX =
      sceneRect().width() - frameRateTextItem_->boundingRect().width() - 10;
  int posY =
      sceneRect().height() - frameRateTextItem_->boundingRect().height() - 10;
  frameRateItem_->setPos(posX, posY);
}

void ImageInspectorGraphicsScene::deleteChildInspector(QGraphicsItem* item) {
  // Delete the selected item
  if (item != nullptr) {
    if (item->group() != nullptr) {
      removeItem(item->group());
      markersList.remove(item->group());
    } else {
      removeItem(item);
      markersList.remove(item);
    }
  }
}

void ImageInspectorGraphicsScene::deleteAllMarkers() {
  for (auto& item : markersList) {
    removeItem(item);
  }
  markersList.clear();
}

void ImageInspectorGraphicsScene::saveFrame() {
  char filename[256];
  time_t frameTime = chrono::system_clock::to_time_t(lastFrameTime_);
  tm* tm = localtime(&frameTime);
  snprintf(filename, sizeof(filename), "%d-%02d-%02d %02d:%02d:%02d.%03d.png",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
           tm->tm_min, tm->tm_sec,
           chrono::duration_cast<chrono::milliseconds>(
               lastFrameTime_.time_since_epoch())
                   .count() %
               1000);
  QImage image(this->sceneRect().size().toSize(), QImage::Format_ARGB32);
  image.fill(Qt::transparent);
  QPainter painter(&image);
  this->render(&painter);
  image.save(filename);
}

void ImageInspectorGraphicsScene::addRowScanner(int row_y) {
  cout << "addRowScanner triggered" << endl;
}
void ImageInspectorGraphicsScene::addColScanner(int col_x) {
  cout << "addColScanner triggered" << endl;
}
// void ImageInspectorGraphicsScene::addHistROI() {
//   cout << "addHistROI triggered" << endl;
// }
void ImageInspectorGraphicsScene::addTrackerPoint(QPoint point) {
  cout << "addTrackerPoint triggered" << endl;
  time_t frameTime = chrono::system_clock::to_time_t(lastFrameTime_);
  tm* tm = localtime(&frameTime);
  float depthValue_ = viewer_->depthMap_.at<float>(point.y(), point.x());
  addCrossHair(point, depthValue_);
  distance_logger_->info("[{}-{}-{} {}:{}:{}.{}] [x,y]=[{},{}] depth={}",
                         tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                         tm->tm_hour, tm->tm_min, tm->tm_sec,
                         chrono::duration_cast<chrono::milliseconds>(
                             lastFrameTime_.time_since_epoch())
                                 .count() %
                             1000,
                         point.x(), point.y(), depthValue_);
}

VideoSinkViewer::VideoSinkViewer(const string& name, QWidget* parent)
    : QWidget(parent), BaseSink(name) {
  layout_ = new QVBoxLayout(this);
  scene_ = new ImageInspectorGraphicsScene(this);
  scene_->viewer_ = this;
  view_ = new QGraphicsView(scene_, this);

  view_->setContentsMargins(0, 0, 0, 0);
  layout_->setSpacing(0);
  layout_->setContentsMargins(0, 0, 0, 0);
  layout_->addWidget(view_);

  // even though newPixMap and onNewPixMap belongs to the same class, they need
  // to be connected using signal/slot mechanism because they are called in
  // different thread.
  connect(this, &VideoSinkViewer::newPixMap, this,
          &VideoSinkViewer::onNewPixMap);

  setWindowFlags(Qt::WindowStaysOnTopHint);

  channel_ = ViewerChannel::Depth;
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
  // double min, max;
  // cv::minMaxLoc(depth, &min, &max);
  // logger_->info("min={}, max={}", min, max);
  depthMap_ = depth;
  Mat depth_norm;
  cv::normalize(depth, depth_norm, 0, 255, cv::NORM_MINMAX, CV_8UC1);
  if (channel_ == ViewerChannel::Depth) {
    cv::applyColorMap(depth_norm, depth_norm, colorMapStyle_);
    cv::cvtColor(depth_norm, depth_norm, cv::COLOR_BGR2RGB);
  } else {
    cv::cvtColor(depth_norm, depth_norm, cv::COLOR_GRAY2RGB);
  }
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

void VideoSinkViewer::SetColorMapStyle(int style) { colorMapStyle_ = style; }