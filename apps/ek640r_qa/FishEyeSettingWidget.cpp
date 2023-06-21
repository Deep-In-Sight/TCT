#include "FishEyeSettingWidget.hpp"

#include <QFormLayout>

FishEyeSettingWidget::FishEyeSettingWidget(QWidget* parent) : QWidget(parent) {
  auto layout = new QFormLayout(this);
  enableCheckBox_ = new QCheckBox(this);
  fxEdit_ = new QLineEdit(this);
  fyEdit_ = new QLineEdit(this);
  cxEdit_ = new QLineEdit(this);
  cyEdit_ = new QLineEdit(this);
  k1Edit_ = new QLineEdit(this);
  k2Edit_ = new QLineEdit(this);
  k3Edit_ = new QLineEdit(this);
  p1Edit_ = new QLineEdit(this);
  p2Edit_ = new QLineEdit(this);
  upscaleEdit_ = new QLineEdit(this);
  enableCheckBox_->setChecked(true);
  fxEdit_->setText("390.33216865");
  fyEdit_->setText("392.37363314");
  cxEdit_->setText("337.7728288");
  cyEdit_->setText("244.44454662");
  k1Edit_->setText("-0.34335484");
  k2Edit_->setText("0.14217051");
  k3Edit_->setText("-0.03051488");
  p1Edit_->setText("0.00073704");
  p2Edit_->setText("-0.00061519");
  upscaleEdit_->setText("4.0");
  layout->addRow("Enable", enableCheckBox_);
  layout->addRow("fx", fxEdit_);
  layout->addRow("fy", fyEdit_);
  layout->addRow("cx", cxEdit_);
  layout->addRow("cy", cyEdit_);
  layout->addRow("k1", k1Edit_);
  layout->addRow("k2", k2Edit_);
  layout->addRow("p1", p1Edit_);
  layout->addRow("p2", p2Edit_);
  layout->addRow("k3", k3Edit_);
  layout->addRow("up", upscaleEdit_);

  connect(enableCheckBox_, &QCheckBox::stateChanged, this,
          &FishEyeSettingWidget::onEnableStateChanged);

  connect(fxEdit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
  connect(fyEdit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
  connect(cxEdit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
  connect(cyEdit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
  connect(k1Edit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
  connect(k2Edit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
  connect(k3Edit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
  connect(p1Edit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
  connect(p2Edit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
  connect(upscaleEdit_, &QLineEdit::textChanged, this,
          &FishEyeSettingWidget::onTextChanged);
}

FishEyeSettingWidget::~FishEyeSettingWidget() {}

void FishEyeSettingWidget::SetFishEye(Fisheye* fisheye) {
  fisheye_ = fisheye;
  onTextChanged("");
}

void FishEyeSettingWidget::onTextChanged(const QString& text) {
  FisheyeParams params;
  params.fx = fxEdit_->text().toFloat();
  params.fy = fyEdit_->text().toFloat();
  params.cx = cxEdit_->text().toFloat();
  params.cy = cyEdit_->text().toFloat();
  params.k1 = k1Edit_->text().toFloat();
  params.k2 = k2Edit_->text().toFloat();
  params.k3 = k3Edit_->text().toFloat();
  params.p1 = p1Edit_->text().toFloat();
  params.p2 = p2Edit_->text().toFloat();
  params.upscale = upscaleEdit_->text().toFloat();
  fisheye_->SetParams(params);
}

void FishEyeSettingWidget::onEnableStateChanged(int state) {
  fisheye_->SetEnable(state == Qt::Checked);
}