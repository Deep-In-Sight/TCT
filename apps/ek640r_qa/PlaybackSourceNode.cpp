#include "PlaybackSourceNode.hpp"

#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include <QtWidgets/QLineEdit>

#include "ToFDataTypes.hpp"

PlaybackSourceNode::PlaybackSourceNode()
    : NodeBase(kSourceNode, "PlaybackSource0", "PlaybackSource", true) {
  _source = new PlaybackSource("", false, false);
  _settingWidget = new PlaybackSettingWidget();
  _settingWidget->setPlaybackSource(_source);
  setElement(_source);
}

PlaybackSourceNode::~PlaybackSourceNode() {
  delete _source;
  delete _settingWidget;
}

QJsonObject PlaybackSourceNode::save() const {
  QJsonObject modelJson = NodeDelegateModel::save();

  modelJson["filename"] = _settingWidget->_lineEditFilePath->text();
  modelJson["fps"] = _settingWidget->_spinBoxFps->value();
  modelJson["loop"] = _settingWidget->_checkboxLoop->checkState();
  modelJson["width"] = _settingWidget->_lineEditWidth->text().toInt();
  modelJson["height"] = _settingWidget->_lineEditHeight->text().toInt();
  return modelJson;
}

void PlaybackSourceNode::load(QJsonObject const& p) {
  NodeDelegateModel::load(p);

  _settingWidget->_lineEditFilePath->setText(p["filename"].toString());
  _settingWidget->_spinBoxFps->setValue(p["fps"].toDouble());
  _settingWidget->_checkboxLoop->setCheckState(
      static_cast<Qt::CheckState>(p["loop"].toInt()));
  _settingWidget->_lineEditWidth->setText(QString::number(p["width"].toInt()));
  _settingWidget->_lineEditHeight->setText(
      QString::number(p["height"].toInt()));
}

NodeDataType PlaybackSourceNode::dataType(PortType, PortIndex) const {
  return RawData().type();
}

std::shared_ptr<NodeData> PlaybackSourceNode::outData(PortIndex) {
  return std::make_shared<RawData>();
}

QWidget* PlaybackSourceNode::embeddedWidget() { return _settingWidget; }