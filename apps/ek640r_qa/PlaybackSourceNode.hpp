#pragma once

#include <sdk/tof/playback-src.h>

#include <QtCore/QObject>
#include <QtNodes/NodeDelegateModel>
#include <iostream>

#include "PlaybackSettingWidget.hpp"

class DecimalData;

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;

/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class PlaybackSourceNode : public NodeDelegateModel {
  Q_OBJECT

 public:
  PlaybackSourceNode();

  virtual ~PlaybackSourceNode();

 public:
  QString caption() const override;

  bool captionVisible() const override;

  QString name() const override;

 public:
  unsigned int nPorts(PortType portType) const override;

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData> outData(PortIndex port) override;

  void setInData(std::shared_ptr<NodeData>, PortIndex) override;

  QWidget *embeddedWidget() override;

  Pad *getPad();

 private:
  PlaybackSource *_source;
  PlaybackSettingWidget *_widget;
};
