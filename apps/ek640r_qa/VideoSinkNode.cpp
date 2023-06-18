#include "VideoSinkNode.hpp"

#include "ToFDataTypes.hpp"

VideoSinkNode::VideoSinkNode()
    : NodeBase(kSinkNode, "VideoSink0", "VideoSink", true) {
  // _sink = new ImageInspectorGraphicsScene();
  _sink = new VideoSink();
  setElement(_sink);
}

NodeDataType VideoSinkNode::dataType(PortType, PortIndex) const {
  return DepthAmplitudeData().type();
}

std::shared_ptr<NodeData> VideoSinkNode::outData(PortIndex) {
  return std::make_shared<DepthAmplitudeData>();
}

QWidget* VideoSinkNode::embeddedWidget() { return nullptr; }