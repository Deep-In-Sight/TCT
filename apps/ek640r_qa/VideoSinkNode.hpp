#pragma once

#include <sdk/core/base-sink.h>

#include <QtCore/QObject>
#include <iostream>

// #include "ImageInspectorGraphicsScene.hpp"
#include "NodeBase.hpp"

class VideoSink : public BaseSink {
 public:
  VideoSink() = default;

  virtual ~VideoSink() = default;

 public:
  void SinkFrame(Mat &frame) override {
    // do nothing
    auto size = frame.size;
    cout << "VideoSink::SinkFrame " << size[0] << "x" << size[1] << "x"
         << size[2] << endl;
  };
};

class VideoSinkNode : public NodeBase {
  Q_OBJECT

 public:
  VideoSinkNode();

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData> outData(PortIndex port) override;

  QWidget *embeddedWidget() override;

 private:
  VideoSink *_sink;
};
