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

class ImageInspectorGraphicsScene : public QGraphicsScene, public BaseSink {
 public:
  ImageInspectorGraphicsScene(QWidget* parent = nullptr);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

  void SinkFrame(Mat& frame) override;

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
};

#endif