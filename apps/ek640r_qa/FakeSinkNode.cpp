#include "FakeSinkNode.hpp"

#include "ToFDataTypes.hpp"

FakeSinkNode::FakeSinkNode() {
  _sink = new FakeSink();
  setNodePrivate(_sink);
}

FakeSinkNode::~FakeSinkNode() {}

QString FakeSinkNode::caption() const { return QStringLiteral("FakeSink"); }

bool FakeSinkNode::captionVisible() const { return true; }

QString FakeSinkNode::name() const { return QStringLiteral("FakeSink0"); }

unsigned int FakeSinkNode::nPorts(PortType portType) const {
  unsigned int result = 1;

  switch (portType) {
    case PortType::In:
      result = 1;
      break;

    case PortType::Out:
      result = 0;

    default:
      break;
  }

  return result;
}

NodeDataType FakeSinkNode::dataType(PortType, PortIndex) const {
  return AnyData().type();
}

std::shared_ptr<NodeData> FakeSinkNode::outData(PortIndex) {
  return std::make_shared<AnyData>();
}

void FakeSinkNode::setInData(std::shared_ptr<NodeData> data, PortIndex) {}

QWidget* FakeSinkNode::embeddedWidget() { return nullptr; }

Pad* FakeSinkNode::getPad() { return _sink->GetSinkPad(); }