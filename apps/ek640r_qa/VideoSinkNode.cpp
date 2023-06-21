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

  auto layout3 = new QHBoxLayout();
  layout3->addWidget(new QLabel("ColorMap"));
  comboBoxColorMapStyle_ = new QComboBox();
  comboBoxColorMapStyle_->addItem("AUTUMN");
  comboBoxColorMapStyle_->addItem("BONE");
  comboBoxColorMapStyle_->addItem("JET");
  comboBoxColorMapStyle_->addItem("WINTER");
  comboBoxColorMapStyle_->addItem("RAINBOW");
  comboBoxColorMapStyle_->addItem("OCEAN");
  comboBoxColorMapStyle_->addItem("SUMMER");
  comboBoxColorMapStyle_->addItem("SPRING");
  comboBoxColorMapStyle_->addItem("COOL");
  comboBoxColorMapStyle_->addItem("HSV");
  comboBoxColorMapStyle_->addItem("PINK");
  comboBoxColorMapStyle_->addItem("HOT");
  comboBoxColorMapStyle_->addItem("PARULA");
  comboBoxColorMapStyle_->addItem("MAGMA");
  comboBoxColorMapStyle_->addItem("INFERNO");
  comboBoxColorMapStyle_->addItem("PLASMA");
  comboBoxColorMapStyle_->addItem("VIRIDIS");
  comboBoxColorMapStyle_->addItem("CIVIDIS");
  comboBoxColorMapStyle_->addItem("TWILIGHT");
  comboBoxColorMapStyle_->addItem("TWILIGHT_SHIFTED");
  comboBoxColorMapStyle_->addItem("TURBO");
  comboBoxColorMapStyle_->addItem("DEEPGREEN");
  comboBoxColorMapStyle_->setCurrentIndex(0);
  layout3->addWidget(comboBoxColorMapStyle_);

  auto layout = new QVBoxLayout(this);
  layout->addLayout(layout1);
  layout->addLayout(layout2);
  layout->addLayout(layout3);

  connect(checkboxEnableViewer_, &QCheckBox::stateChanged, this,
          &ViewerConfigWidget::onEnableViewerChanged);
  connect(comboBoxViewerType_, qOverload<int>(&QComboBox::currentIndexChanged),
          this, &ViewerConfigWidget::onViewerTypeChanged);
  connect(comboBoxColorMapStyle_, qOverload<int>(&QComboBox::currentIndexChanged),
          this, &ViewerConfigWidget::onViewerColorMapStyleChanged);
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

void ViewerConfigWidget::onViewerColorMapStyleChanged(int index) {
  if (viewer_ == nullptr) {
    return;
  }

  viewer_->SetColorMapStyle(index);
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
  modelJson["viewer_colormap"] =
      configWidget_->comboBoxColorMapStyle_->currentIndex();
  return modelJson;
}

void VideoSinkNode::load(QJsonObject const& p) {
  NodeBase::load(p);
  configWidget_->checkboxEnableViewer_->setChecked(
      p["viewer_enabled"].toBool());
  configWidget_->comboBoxViewerType_->setCurrentIndex(p["viewer_type"].toInt());
  configWidget_->comboBoxColorMapStyle_->setCurrentIndex(
      p["viewer_colormap"].toInt());
}

NodeDataType VideoSinkNode::dataType(PortType, PortIndex) const {
  return DepthAmplitudeData().type();
}

std::shared_ptr<NodeData> VideoSinkNode::outData(PortIndex) {
  return std::make_shared<DepthAmplitudeData>();
}

QWidget* VideoSinkNode::embeddedWidget() { return configWidget_; }