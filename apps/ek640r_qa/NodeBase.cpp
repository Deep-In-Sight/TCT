#include "NodeBase.hpp"

NodeBase::NodeBase(NodeType type, const string &caption, const string &nodeName,
                   bool captionVisible) {
  type_ = type;
  element_ = nullptr;
  caption_ = caption;
  name_ = nodeName;
  captionVisible_ = captionVisible;
}

QString NodeBase::caption() const { return QString::fromStdString(caption_); }

bool NodeBase::captionVisible() const { return captionVisible_; }

QString NodeBase::name() const { return QString::fromStdString(name_); }

unsigned int NodeBase::nPorts(QtNodes::PortType portType) const {
  unsigned int result = 1;

  switch (portType) {
    case QtNodes::PortType::In:
      if (type_ == kSourceNode) {
        result = 0;
      } else {
        result = 1;
      }
      break;

    case QtNodes::PortType::Out:
      if (type_ == kSinkNode) {
        result = 0;
      } else {
        result = 1;
      }

    default:
      break;
  }

  return result;
}

void NodeBase::setInData(std::shared_ptr<QtNodes::NodeData> data,
                         QtNodes::PortIndex) {}

void NodeBase::setElement(Element *element) { element_ = element; }

Element *NodeBase::getElement() { return element_; }