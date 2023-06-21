#ifndef __FISH_EYE_SESTTING_WIDGET_HPP__
#define __FISH_EYE_SESTTING_WIDGET_HPP__

#include <sdk/calib/fisheye.h>

#include <QCheckBox>
#include <QLineEdit>
#include <QWidget>

class FishEyeSettingWidget : public QWidget {
  Q_OBJECT

 public:
  FishEyeSettingWidget(QWidget *parent = nullptr);
  ~FishEyeSettingWidget();

  void SetFishEye(Fisheye *fisheye);

 private slots:
  void onTextChanged(const QString &text);
  void onEnableStateChanged(int state);

 public:
  QCheckBox *enableCheckBox_;
  QLineEdit *fxEdit_;
  QLineEdit *fyEdit_;
  QLineEdit *cxEdit_;
  QLineEdit *cyEdit_;
  QLineEdit *k1Edit_;
  QLineEdit *k2Edit_;
  QLineEdit *k3Edit_;
  QLineEdit *p1Edit_;
  QLineEdit *p2Edit_;
  QLineEdit *upscaleEdit_;
  Fisheye *fisheye_;
};

#endif  //__FISH_EYE_SESTTING_WIDGET_HPP__