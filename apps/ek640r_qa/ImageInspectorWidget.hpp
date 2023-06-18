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

class ImageInspectorGraphicsScene : public QGraphicsScene {
  Q_OBJECT
 public:
  ImageInspectorGraphicsScene(QWidget* parent = nullptr);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

  void SetImage(const QPixmap& pixmap);

 private slots:
  void deleteChildInspector();

  void addRowScanner();
  void addColScanner();
  void addHistROI();
  void addTrackerPoint();

 private:
  QMenu* menuAddInspector_;
  QMenu* menuDeleteInspector_;
  QGraphicsItem* selectedItem_;
  QPointF lastPos_;
  QGraphicsPixmapItem* pixmapItem_;
};

class VideoSinkViewer : public QWidget, public BaseSink {
  Q_OBJECT

 public:
  enum class ViewerChannel { Depth, Amplitude };
  VideoSinkViewer(QWidget* parent = nullptr);
  ~VideoSinkViewer();

  void SinkFrame(Mat& frame) override;
  void SetChannel(ViewerChannel channel);

 signals:
  void newPixMap(const QPixmap& pixmap);

 public slots:
  void onNewPixMap(const QPixmap& pixmap);

 private:
  QVBoxLayout* layout_;
  QGraphicsView* view_;
  ImageInspectorGraphicsScene* scene_;
  ViewerChannel channel_;
};

#endif