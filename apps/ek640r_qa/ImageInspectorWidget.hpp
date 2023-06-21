#ifndef __IMAGE_INSPECTOR_GRAPHICS_SCENE_H__
#define __IMAGE_INSPECTOR_GRAPHICS_SCENE_H__

#include <sdk/core/base-sink.h>

#include <QGraphicsLineItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>

using namespace std;

class VideoSinkViewer;

class ImageInspectorGraphicsScene : public QGraphicsScene {
  Q_OBJECT
 public:
  ImageInspectorGraphicsScene(QWidget* parent = nullptr);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

  void SetImage(const QPixmap& pixmap);

 private slots:
  void deleteChildInspector(QGraphicsItem* item);
  void deleteAllMarkers();
  void saveFrame();

  void addRowScanner(int row_y);
  void addColScanner(int col_x);
  // void addHistROI(QRect rect);
  void addTrackerPoint(QPoint point);

 private:
  void addCrossHair(QPoint point, float depthVal_);
  void addFrameRate(float frameRate);

 public:
  QMenu* menuInspector_;
  QMenu* menuDeleteInspector_;
  QGraphicsItem* selectedItem_;
  QPointF lastPos_;
  QGraphicsPixmapItem* pixmapItem_;
  QGraphicsItemGroup* frameRateItem_;
  chrono::time_point<chrono::system_clock> lastFrameTime_;
  VideoSinkViewer* viewer_;
  list<QGraphicsItem*> markersList;
};

class VideoSinkViewer : public QWidget, public BaseSink {
  Q_OBJECT

 public:
  enum class ViewerChannel { Depth, Amplitude };
  VideoSinkViewer(QWidget* parent = nullptr);
  ~VideoSinkViewer();

  void SinkFrame(Mat& frame) override;
  void SetChannel(ViewerChannel channel);
  void SetColorMapStyle(int style);

 signals:
  void newPixMap(const QPixmap& pixmap);

 public slots:
  void onNewPixMap(const QPixmap& pixmap);

 public:
  QVBoxLayout* layout_;
  QGraphicsView* view_;
  ImageInspectorGraphicsScene* scene_;
  ViewerChannel channel_;
  Mat depthMap_;
  int colorMapStyle_;
};

#endif