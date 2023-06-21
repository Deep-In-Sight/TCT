#include "FishEyeNode.hpp"

#include "ToFDataTypes.hpp"

FishEyeNode::FishEyeNode()
    : NodeBase(kTransformNode, "Fisheye0", "Fisheye", true) {
  fisheye_ = new Fisheye("fisheye");
  settingWidget_ = new FishEyeSettingWidget();
  settingWidget_->SetFishEye(fisheye_);
  setElement(fisheye_);
}

FishEyeNode::~FishEyeNode() { delete fisheye_; }

QJsonObject FishEyeNode::save() const {
  QJsonObject modelJson = NodeBase::save();
  modelJson["enable"] = settingWidget_->enableCheckBox_->isChecked();
  modelJson["fx"] = settingWidget_->fxEdit_->text().toDouble();
  modelJson["fy"] = settingWidget_->fyEdit_->text().toDouble();
  modelJson["cx"] = settingWidget_->cxEdit_->text().toDouble();
  modelJson["cy"] = settingWidget_->cyEdit_->text().toDouble();
  modelJson["k1"] = settingWidget_->k1Edit_->text().toDouble();
  modelJson["k2"] = settingWidget_->k2Edit_->text().toDouble();
  modelJson["p1"] = settingWidget_->p1Edit_->text().toDouble();
  modelJson["p2"] = settingWidget_->p2Edit_->text().toDouble();
  modelJson["k3"] = settingWidget_->k3Edit_->text().toDouble();
  modelJson["upscale"] = settingWidget_->upscaleEdit_->text().toDouble();
  return modelJson;
}

void FishEyeNode::load(QJsonObject const &modelJson) {
  NodeBase::load(modelJson);
  settingWidget_->enableCheckBox_->setChecked(modelJson["enable"].toBool());
  settingWidget_->fxEdit_->setText(QString::number(modelJson["fx"].toDouble()));
  settingWidget_->fyEdit_->setText(QString::number(modelJson["fy"].toDouble()));
  settingWidget_->cxEdit_->setText(QString::number(modelJson["cx"].toDouble()));
  settingWidget_->cyEdit_->setText(QString::number(modelJson["cy"].toDouble()));
  settingWidget_->k1Edit_->setText(QString::number(modelJson["k1"].toDouble()));
  settingWidget_->k2Edit_->setText(QString::number(modelJson["k2"].toDouble()));
  settingWidget_->k3Edit_->setText(QString::number(modelJson["k3"].toDouble()));
  settingWidget_->p1Edit_->setText(QString::number(modelJson["p1"].toDouble()));
  settingWidget_->p2Edit_->setText(QString::number(modelJson["p2"].toDouble()));
  settingWidget_->upscaleEdit_->setText(
      QString::number(modelJson["upscale"].toDouble()));
}

NodeDataType FishEyeNode::dataType(PortType portType,
                                   PortIndex portIndex) const {
  return DepthAmplitudeData().type();
}

std::shared_ptr<NodeData> FishEyeNode::outData(PortIndex port) {
  return std::make_shared<DepthAmplitudeData>();
}

QWidget *FishEyeNode::embeddedWidget() { return settingWidget_; }