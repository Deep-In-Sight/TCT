#include "MovingAverageNode.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QObject>

#include "ToFDataTypes.hpp"

MovingAverageConfigWidget::MovingAverageConfigWidget(QWidget *parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->addWidget(new QLabel("Window Size"));
  spinBoxWindowSize_ = new QSpinBox(this);
  spinBoxWindowSize_->setRange(1, 64);
  spinBoxWindowSize_->setValue(4);
  layout->addWidget(spinBoxWindowSize_);
  connect(spinBoxWindowSize_, qOverload<int>(&QSpinBox::valueChanged), this,
          &MovingAverageConfigWidget::onWindowSizeChanged);
}

void MovingAverageConfigWidget::SetTransformer(MovingAverage *transformer) {
  transformer_ = transformer;
}

void MovingAverageConfigWidget::onWindowSizeChanged(int value) {
  transformer_->SetWindowSize(value);
}

MovingAverageNode::MovingAverageNode()
    : NodeBase(kTransformNode, "MovingAverage0", "MovingAverage", true) {
  transformer_ = new MovingAverage("MovingAverage");
  configWidget_ = new MovingAverageConfigWidget();
  configWidget_->SetTransformer(transformer_);
  setElement(transformer_);
}

QJsonObject MovingAverageNode::save() const {
  QJsonObject modelJson = NodeBase::save();
  modelJson["windowSize"] = configWidget_->spinBoxWindowSize_->value();
  return modelJson;
}

void MovingAverageNode::load(QJsonObject const &p) {
  NodeBase::load(p);
  configWidget_->spinBoxWindowSize_->setValue(p["windowSize"].toInt());
}

NodeDataType MovingAverageNode::dataType(PortType type, PortIndex index) const {
  if (type == PortType::In) {
    return DepthAmplitudeData().type();
  } else {
    return DepthAmplitudeData().type();
  }
}

std::shared_ptr<NodeData> MovingAverageNode::outData(PortIndex index) {
  return std::make_shared<DepthAmplitudeData>();
}

QWidget *MovingAverageNode::embeddedWidget() { return configWidget_; }