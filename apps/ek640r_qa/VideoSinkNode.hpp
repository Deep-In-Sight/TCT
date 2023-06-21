#pragma once

#include <sdk/core/base-sink.h>

#include <QtCore/QObject>
#include <iostream>

// #include "ImageInspectorGraphicsScene.hpp"
#include <QCheckBox>
#include <QComboBox>

#include "ImageInspectorWidget.hpp"
#include "NodeBase.hpp"

class ViewerConfigWidget : public QWidget {
  Q_OBJECT
 public:
  ViewerConfigWidget(QWidget* parent = nullptr);
  ~ViewerConfigWidget();

  void SetViewer(VideoSinkViewer* viewer);

 private:
  VideoSinkViewer* viewer_;

 private slots:
  void onEnableViewerChanged(int state);
  void onViewerTypeChanged(int index);
  void onViewerColorMapStyleChanged(int index);

 public:
  QCheckBox* checkboxEnableViewer_;
  QComboBox* comboBoxViewerType_;
  QComboBox* comboBoxColorMapStyle_;
};

class VideoSinkNode : public NodeBase {
  Q_OBJECT

 public:
  VideoSinkNode();

  QJsonObject save() const override;

  void load(QJsonObject const& p) override;

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData> outData(PortIndex port) override;

  QWidget* embeddedWidget() override;

 private:
  VideoSinkViewer* _sink;
  ViewerConfigWidget* configWidget_;
};
