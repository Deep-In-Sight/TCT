#include "CameraSourceSettingWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
CameraSourceSettingWidget::CameraSourceSettingWidget(QWidget *parent)
    : QWidget(parent) {
  auto layout0 = new QHBoxLayout();
  layout0->addWidget(new QLabel("device"));
  lineEditDevice_ = new QLineEdit();
  lineEditDevice_->setText("192.168.7.2:50660");
  layout0->addWidget(lineEditDevice_);

  auto layout1 = new QHBoxLayout();
  layout1->addWidget(new QLabel("fmodMhz"));
  spinboxFmod_ = new QSpinBox();
  spinboxFmod_->setRange(0, 120);
  spinboxFmod_->setSingleStep(1);
  spinboxFmod_->setValue(37);
  layout1->addWidget(spinboxFmod_);

  auto layout2 = new QHBoxLayout();
  buttonPlayStop_ = new QPushButton("Play");
  layout2->addWidget(buttonPlayStop_);

  auto layout = new QVBoxLayout(this);
  layout->addLayout(layout0);
  layout->addLayout(layout1);
  layout->addLayout(layout2);

  connect(lineEditDevice_, &QLineEdit::textChanged, this,
          &CameraSourceSettingWidget::onDeviceChanged);

  connect(spinboxFmod_, qOverload<int>(&QSpinBox::valueChanged), this,
          &CameraSourceSettingWidget::onFmodChanged);

  connect(buttonPlayStop_, &QPushButton::clicked, this,
          &CameraSourceSettingWidget::onPlayStopClicked);
}

void CameraSourceSettingWidget::SetSource(ToFCameraSrc *source) {
  source_ = source;
}

void CameraSourceSettingWidget::onDeviceChanged(const QString &text) {
  if (source_) {
    source_->SetDeviceName(text.toStdString());
  }
}
void CameraSourceSettingWidget::onFmodChanged(int value) {
  if (source_) {
    source_->SetFmod(value);
  }
}

void CameraSourceSettingWidget::onPlayStopClicked() {
  if (source_) {
    if (source_->GetState() == kStreamStatePlaying) {
      source_->Stop();
      buttonPlayStop_->setText("Play");
    } else if (source_->GetState() == kStreamStateStopped) {
      source_->SetDeviceName(lineEditDevice_->text().toStdString());
      source_->SetFmod(spinboxFmod_->value());
      source_->Start();
      buttonPlayStop_->setText("Stop");
    }
  }
}