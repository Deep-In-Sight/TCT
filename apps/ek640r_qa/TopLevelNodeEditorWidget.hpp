#ifndef _TOP_LEVEL_NODE_EDITOR_WIDGET_HPP_
#define _TOP_LEVEL_NODE_EDITOR_WIDGET_HPP_

#include <QLabel>
#include <QWidget>
#include <QtGui/QScreen>
#include <QtNodes/ConnectionStyle>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QVBoxLayout>

#include "CameraSourceNode.hpp"
#include "DepthCalcNode.hpp"
#include "MovingAverageNode.hpp"
#include "PipelineModel.hpp"
#include "PlaybackSourceNode.hpp"
#include "VideoSinkNode.hpp"

using QtNodes::ConnectionStyle;
using QtNodes::DataFlowGraphicsScene;
using QtNodes::DataFlowGraphModel;
using QtNodes::GraphicsView;
using QtNodes::NodeDelegateModelRegistry;

class TopLevelNodeEditorWidget : public QWidget {
  Q_OBJECT
 public:
  TopLevelNodeEditorWidget(QWidget *parent = nullptr);

  void resizeEvent(QResizeEvent *event) override;

 private:
  PipelineModel *dataFlowGraphModel;
  QLabel *labelLogo;
};

#endif