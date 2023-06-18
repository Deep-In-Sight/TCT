#ifndef __DEPTH_CALC_NODE_HPP__
#define __DEPTH_CALC_NODE_HPP__

#include <sdk/tof/depth-calc.h>

#include "DepthCalcConfigWidget.hpp"
#include "NodeBase.hpp"

class DepthCalcNode : public NodeBase {
  Q_OBJECT

 public:
  DepthCalcNode();

  QJsonObject save() const override;
  void load(QJsonObject const &p) override;

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData> outData(PortIndex port) override;

  QWidget *embeddedWidget() override;

 private:
  DepthCalc *transformer_;
  DepthCalcConfigWidget *configWidget_;
};

#endif  //__DEPTH_CALC_NODE_HPP__