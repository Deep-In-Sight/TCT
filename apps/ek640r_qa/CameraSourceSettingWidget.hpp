#ifndef __CAMERA_SOURCE_SETTING_WIDGET_HPP__
#define __CAMERA_SOURCE_SETTING_WIDGET_HPP__

#include <sdk/tof/camera-src.h>

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

class CameraSourceSettingWidget : public QWidget {
  Q_OBJECT

 public:
  CameraSourceSettingWidget(QWidget *parent = nullptr);

  void SetSource(ToFCameraSrc *source);

 private slots:
  void onDeviceChanged(const QString &text);
  void onFmodChanged(int value);
  void onPlayStopClicked();
  void onShortCaptureClicked();

 public:
  QLineEdit *lineEditDevice_;
  QSpinBox *spinboxFmod_;
  QSpinBox *spinboxExposureTime_;
  QPushButton *buttonPlayStop_;
  QPushButton *buttonLongExposure_;

  ToFCameraSrc *source_;
};

#endif  //__CAMERA_SOURCE_SETTING_WIDGET_HPP__