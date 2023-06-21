#include "CameraSourceNode.hpp"

#include "ToFDataTypes.hpp"

CameraSourceNode::CameraSourceNode()
    : NodeBase(kSourceNode, "CameraSource0", "CameraSource", true) {
  source_ = new ToFCameraSrc("");
  settingWidget_ = new CameraSourceSettingWidget();
  settingWidget_->SetSource(source_);
  setElement(source_);
}

CameraSourceNode::~CameraSourceNode() {
  delete source_;
  delete settingWidget_;
}

QJsonObject CameraSourceNode::save() const {
  QJsonObject modelJson = NodeDelegateModel::save();

  modelJson["device"] = settingWidget_->lineEditDevice_->text();
  modelJson["fmod"] = settingWidget_->spinboxFmod_->value();
  modelJson["exposureTime"] = settingWidget_->spinboxExposureTime_->value();

  return modelJson;
}

void CameraSourceNode::load(QJsonObject const& p) {
  NodeDelegateModel::load(p);

  settingWidget_->lineEditDevice_->setText(p["device"].toString());
  settingWidget_->spinboxFmod_->setValue(p["fmod"].toInt());
  settingWidget_->spinboxExposureTime_->setValue(p["exposureTime"].toInt());
}

NodeDataType CameraSourceNode::dataType(PortType type, PortIndex index) const {
  return RawData().type();
}

std::shared_ptr<NodeData> CameraSourceNode::outData(PortIndex index) {
  return std::make_shared<RawData>();
}

QWidget* CameraSourceNode::embeddedWidget() { return settingWidget_; }
