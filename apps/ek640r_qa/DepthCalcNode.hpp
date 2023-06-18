#include <sdk/tof/depth-calc.h>

#include "DepthCalcConfigWidget.hpp"
#include "NodeBase.hpp"

class DepthCalcNode : public NodeBase {
  Q_OBJECT

 public:
  DepthCalcNode();

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData> outData(PortIndex port) override;

  QWidget *embeddedWidget() override;

 private:
  DepthCalc *transformer_;
  DepthCalcConfigWidget *configWidget_;
};
