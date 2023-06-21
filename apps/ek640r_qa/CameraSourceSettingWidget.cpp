#include "CameraSourceSettingWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

CameraSourceSettingWidget::CameraSourceSettingWidget(QWidget *parent)
    : QWidget(parent) {
  auto layout0 = new QHBoxLayout();
  layout0->addWidget(new QLabel("device"));
  lineEditDevice_ = new QLineEdit();
  lineEditDevice_->setText("192.168.7.2:50660");
  layout0->addWidget(lineEditDevice_);

  auto layout1 = new QHBoxLayout();
  layout1->addWidget(new QLabel("fmod(Mhz)"));
  spinboxFmod_ = new QSpinBox();
  spinboxFmod_->setRange(0, 120);
  spinboxFmod_->setSingleStep(1);
  spinboxFmod_->setValue(37);
  layout1->addWidget(spinboxFmod_);

  auto layout2 = new QHBoxLayout();
  layout2->addWidget(new QLabel("exposureTime(s)"));
  spinboxExposureTime_ = new QSpinBox();
  spinboxExposureTime_->setRange(0, 10);
  spinboxExposureTime_->setSingleStep(1);
  spinboxExposureTime_->setValue(5);
  layout2->addWidget(spinboxExposureTime_);

  auto layout3 = new QHBoxLayout();
  buttonPlayStop_ = new QPushButton("Play");
  buttonLongExposure_ = new QPushButton("Long Exposure");
  layout3->addWidget(buttonPlayStop_);
  layout3->addWidget(buttonLongExposure_);

  auto layout = new QVBoxLayout(this);
  layout->addLayout(layout0);
  layout->addLayout(layout1);
  layout->addLayout(layout2);
  layout->addLayout(layout3);

  connect(lineEditDevice_, &QLineEdit::textChanged, this,
          &CameraSourceSettingWidget::onDeviceChanged);

  connect(spinboxFmod_, qOverload<int>(&QSpinBox::valueChanged), this,
          &CameraSourceSettingWidget::onFmodChanged);

  connect(buttonPlayStop_, &QPushButton::clicked, this,
          &CameraSourceSettingWidget::onPlayStopClicked);

  connect(buttonLongExposure_, &QPushButton::clicked, this,
          &CameraSourceSettingWidget::onShortCaptureClicked);
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
      buttonLongExposure_->setDisabled(false);
      buttonPlayStop_->setText("Play");
    } else if (source_->GetState() == kStreamStateStopped) {
      source_->SetDeviceName(lineEditDevice_->text().toStdString());
      source_->SetFmod(spinboxFmod_->value());
      source_->Start();
      buttonLongExposure_->setDisabled(true);
      buttonPlayStop_->setText("Stop");
    }
  }
}

void CameraSourceSettingWidget::onShortCaptureClicked() {
  if (source_) {
    source_->SetDeviceName(lineEditDevice_->text().toStdString());
    source_->SetFmod(spinboxFmod_->value());
    source_->SetDeviceName(lineEditDevice_->text().toStdString());
    buttonPlayStop_->setDisabled(true);
    buttonLongExposure_->setDisabled(true);
    int exposureTime = spinboxExposureTime_->value();
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [=]() {
      source_->Stop();
      buttonPlayStop_->setDisabled(false);
      buttonLongExposure_->setDisabled(false);
      timer->stop();
      delete timer;
    });
    source_->Start();
    timer->start(exposureTime * 1000);
  }
}