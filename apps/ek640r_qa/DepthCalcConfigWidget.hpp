#ifndef __DEPTH_CALC_CONFIG_WIDGET_H__
#define __DEPTH_CALC_CONFIG_WIDGET_H__
#include <sdk/tof/depth-calc.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

class DepthCalcConfigWidget : public QWidget {
  Q_OBJECT

 public:
  DepthCalcConfigWidget(QWidget *parent = nullptr);

  void SetTransformer(DepthCalc *transformer);

 private slots:
  void onConfigChanged(const QString &text);

 public:
  QLineEdit *lineEditFmod_;
  QLineEdit *lineEditOffset_;

  DepthCalc *transformer_;
};

#endif  //__DEPTH_CALC_CONFIG_WIDGET_H__