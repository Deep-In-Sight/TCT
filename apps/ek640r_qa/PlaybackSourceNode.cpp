#include "PlaybackSourceNode.hpp"

#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include <QtWidgets/QLineEdit>

#include "ToFDataTypes.hpp"

PlaybackSourceNode::PlaybackSourceNode() : _widget{nullptr} {
  _source = new PlaybackSource("", false, false);
  _widget = new PlaybackSettingWidget();
  _widget->setPlaybackSource(_source);
  setNodePrivate(_source);
}

PlaybackSourceNode::~PlaybackSourceNode() {
  delete _source;
  delete _widget;
}

QString PlaybackSourceNode::caption() const {
  return QStringLiteral("PlaybackSource");
}

bool PlaybackSourceNode::captionVisible() const { return true; }

QString PlaybackSourceNode::name() const {
  return QStringLiteral("PlaybackSource0");
}

unsigned int PlaybackSourceNode::nPorts(PortType portType) const {
  unsigned int result = 1;

  switch (portType) {
    case PortType::In:
      result = 0;
      break;

    case PortType::Out:
      result = 1;

    default:
      break;
  }

  return result;
}

NodeDataType PlaybackSourceNode::dataType(PortType, PortIndex) const {
  return RawData().type();
}

std::shared_ptr<NodeData> PlaybackSourceNode::outData(PortIndex) {
  return std::make_shared<RawData>();
}

void PlaybackSourceNode::setInData(std::shared_ptr<NodeData>, PortIndex) {}

QWidget* PlaybackSourceNode::embeddedWidget() { return _widget; }

Pad* PlaybackSourceNode::getPad() { return _source->GetSourcePad(); }