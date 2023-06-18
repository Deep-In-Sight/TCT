#ifndef __NODE_BASE_H__
#define __NODE_BASE_H__

#include <sdk/core/element.h>
#include <string.h>

#include <QtNodes/NodeDelegateModel>

enum NodeType { kSourceNode, kTransformNode, kSinkNode };
class NodeBase : public QtNodes::NodeDelegateModel {
  Q_OBJECT
 public:
  NodeBase(NodeType type, const string &caption, const string &name,
           bool captionVisible);

 public:
  QString caption() const override;
  bool captionVisible() const override;
  QString name() const override;

 public:
  unsigned int nPorts(QtNodes::PortType portType) const override;
  void setInData(std::shared_ptr<QtNodes::NodeData>,
                 QtNodes::PortIndex) override;
  void setElement(Element *element);
  Element *getElement();

 private:
  Element *element_;
  NodeType type_;
  string caption_;
  string name_;
  bool captionVisible_;
};

#endif  // __NODE_BASE_H__