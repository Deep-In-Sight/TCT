#include "VideoSinkNode.hpp"

#include <QLabel>

#include "ToFDataTypes.hpp"

ViewerConfigWidget::ViewerConfigWidget(QWidget* parent)
    : QWidget(parent), viewer_(nullptr) {
  auto layout1 = new QHBoxLayout();
  auto label = new QLabel("Enable");
  layout1->addWidget(label);
  checkboxEnableViewer_ = new QCheckBox();
  checkboxEnableViewer_->setChecked(false);
  layout1->addWidget(checkboxEnableViewer_);

  auto layout2 = new QHBoxLayout();
  label = new QLabel("Type");
  layout2->addWidget(label);
  comboBoxViewerType_ = new QComboBox();
  comboBoxViewerType_->addItem("Depth");
  comboBoxViewerType_->addItem("Amplitude");
  comboBoxViewerType_->setCurrentIndex(0);
  layout2->addWidget(comboBoxViewerType_);

  auto layout = new QVBoxLayout(this);
  layout->addLayout(layout1);
  layout->addLayout(layout2);

  connect(checkboxEnableViewer_, &QCheckBox::stateChanged, this,
          &ViewerConfigWidget::onEnableViewerChanged);
  connect(comboBoxViewerType_, qOverload<int>(&QComboBox::currentIndexChanged),
          this, &ViewerConfigWidget::onViewerTypeChanged);
}

ViewerConfigWidget::~ViewerConfigWidget() {}

void ViewerConfigWidget::SetViewer(VideoSinkViewer* viewer) {
  viewer_ = viewer;
}

void ViewerConfigWidget::onEnableViewerChanged(int state) {
  if (viewer_ == nullptr) {
    return;
  }

  if (state == Qt::Checked) {
    viewer_->show();
  } else {
    viewer_->hide();
  }
}

void ViewerConfigWidget::onViewerTypeChanged(int index) {
  if (viewer_ == nullptr) {
    return;
  }

  if (index == 0) {
    viewer_->SetChannel(VideoSinkViewer::ViewerChannel::Depth);
  } else {
    viewer_->SetChannel(VideoSinkViewer::ViewerChannel::Amplitude);
  }
}

VideoSinkNode::VideoSinkNode()
    : NodeBase(kSinkNode, "VideoSink0", "VideoSink", true) {
  // _sink = new ImageInspectorGraphicsScene();
  _sink = new VideoSinkViewer();
  _sink->setWindowTitle("3D SVM");
  configWidget_ = new ViewerConfigWidget();
  configWidget_->SetViewer(_sink);
  setElement(_sink);
}

QJsonObject VideoSinkNode::save() const {
  QJsonObject modelJson = NodeBase::save();
  modelJson["viewer_enabled"] =
      configWidget_->checkboxEnableViewer_->isChecked();
  modelJson["viewer_type"] = configWidget_->comboBoxViewerType_->currentIndex();
  return modelJson;
}

void VideoSinkNode::load(QJsonObject const& p) {
  NodeBase::load(p);
  configWidget_->checkboxEnableViewer_->setChecked(
      p["viewer_enabled"].toBool());
  configWidget_->comboBoxViewerType_->setCurrentIndex(p["viewer_type"].toInt());
}

NodeDataType VideoSinkNode::dataType(PortType, PortIndex) const {
  return DepthAmplitudeData().type();
}

std::shared_ptr<NodeData> VideoSinkNode::outData(PortIndex) {
  return std::make_shared<DepthAmplitudeData>();
}

QWidget* VideoSinkNode::embeddedWidget() { return configWidget_; }