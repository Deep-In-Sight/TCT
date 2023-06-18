#pragma once

#include <sdk/tof/playback-src.h>

#include <QtCore/QObject>
#include <iostream>

#include "NodeBase.hpp"
#include "PlaybackSettingWidget.hpp"

/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class PlaybackSourceNode : public NodeBase {
  Q_OBJECT

 public:
  PlaybackSourceNode();

  virtual ~PlaybackSourceNode();

 public:
  QJsonObject save() const override;

  void load(QJsonObject const &p) override;

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData> outData(PortIndex port) override;

  QWidget *embeddedWidget() override;

 private:
  PlaybackSource *_source;
  PlaybackSettingWidget *_settingWidget;
};
