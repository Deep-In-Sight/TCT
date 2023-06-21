#ifndef __CAMERA_SOURCE_NODE_HPP__
#define __CAMERA_SOURCE_NODE_HPP__

#include <sdk/tof/camera-src.h>

#include "CameraSourceSettingWidget.hpp"
#include "NodeBase.hpp"

class CameraSourceNode : public NodeBase {
  Q_OBJECT

 public:
  CameraSourceNode();
  ~CameraSourceNode();

  QJsonObject save() const override;
  void load(QJsonObject const &p) override;

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData> outData(PortIndex port) override;

  QWidget *embeddedWidget() override;

 private:
  ToFCameraSrc *source_;
  CameraSourceSettingWidget *settingWidget_;
};

#endif  //__CAMERA_SOURCE_NODE_HPP__