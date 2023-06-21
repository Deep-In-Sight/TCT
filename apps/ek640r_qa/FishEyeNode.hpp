#ifndef __FISH_EYE_NODE_HPP__
#define __FISH_EYE_NODE_HPP__

#include <sdk/calib/fisheye.h>

#include "FishEyeSettingWidget.hpp"
#include "NodeBase.hpp"

class FishEyeNode : public NodeBase {
  Q_OBJECT

 public:
  FishEyeNode();
  ~FishEyeNode();

  QJsonObject save() const override;
  void load(QJsonObject const &p) override;

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData> outData(PortIndex port) override;

  QWidget *embeddedWidget() override;

 private:
  Fisheye *fisheye_;
  FishEyeSettingWidget *settingWidget_;
};

#endif