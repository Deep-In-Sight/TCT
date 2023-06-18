#ifndef __MOVING_AVERAGE_NODE_HPP__
#define __MOVING_AVERAGE_NODE_HPP__

#include <sdk/tof/moving-average.h>

#include <QSpinBox>

#include "NodeBase.hpp"

class MovingAverageConfigWidget : public QWidget {
  Q_OBJECT

 public:
  MovingAverageConfigWidget(QWidget *parent = nullptr);

  void SetTransformer(MovingAverage *transformer);

 private slots:
  void onWindowSizeChanged(int value);

 public:
  QSpinBox *spinBoxWindowSize_;

  MovingAverage *transformer_;
};

class MovingAverageNode : public NodeBase {
  Q_OBJECT
 public:
  MovingAverageNode();

  QJsonObject save() const override;
  void load(QJsonObject const &p) override;

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData> outData(PortIndex port) override;

  QWidget *embeddedWidget() override;

 private:
  MovingAverage *transformer_;
  MovingAverageConfigWidget *configWidget_;
};

#endif  //__MOVING_AVERAGE_NODE_HPP__