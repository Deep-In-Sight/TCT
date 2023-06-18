#include "PlaybackSourceNode.hpp"

#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include <QtWidgets/QLineEdit>

#include "ToFDataTypes.hpp"

PlaybackSourceNode::PlaybackSourceNode()
    : NodeBase(kSourceNode, "PlaybackSource0", "PlaybackSource", true) {
  _source = new PlaybackSource("", false, false);
  _settingWidget = new PlaybackSettingWidget();
  _settingWidget->setPlaybackSource(_source);
  setElement(_source);
}

PlaybackSourceNode::~PlaybackSourceNode() {
  delete _source;
  delete _settingWidget;
}

NodeDataType PlaybackSourceNode::dataType(PortType, PortIndex) const {
  return RawData().type();
}

std::shared_ptr<NodeData> PlaybackSourceNode::outData(PortIndex) {
  return std::make_shared<RawData>();
}

QWidget* PlaybackSourceNode::embeddedWidget() { return _settingWidget; }