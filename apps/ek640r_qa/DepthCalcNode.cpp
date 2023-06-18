#include "DepthCalcNode.hpp"

#include "ToFDataTypes.hpp"

DepthCalcNode::DepthCalcNode()
    : NodeBase(kTransformNode, "DepthTransform0", "DepthTransform", true) {
  // _sink = new ImageInspectorGraphicsScene();
  transformer_ = new DepthCalc();
  configWidget_ = new DepthCalcConfigWidget();
  configWidget_->SetTransformer(transformer_);
  setElement(transformer_);
}

QJsonObject DepthCalcNode::save() const {
  QJsonObject modelJson = NodeDelegateModel::save();

  modelJson["fmod"] = configWidget_->lineEditFmod_->text().toFloat();
  modelJson["offset"] = configWidget_->lineEditOffset_->text().toFloat();

  return modelJson;
}

void DepthCalcNode::load(QJsonObject const& p) {
  NodeDelegateModel::load(p);

  configWidget_->lineEditFmod_->setText(QString::number(p["fmod"].toDouble()));
  configWidget_->lineEditOffset_->setText(
      QString::number(p["offset"].toDouble()));
}

NodeDataType DepthCalcNode::dataType(PortType type, PortIndex index) const {
  if (type == PortType::In) {
    return RawData().type();
  } else {
    return DepthAmplitudeData().type();
  }
}

std::shared_ptr<NodeData> DepthCalcNode::outData(PortIndex index) {
  return std::make_shared<DepthAmplitudeData>();
}

QWidget* DepthCalcNode::embeddedWidget() { return configWidget_; }