#include "DepthCalcConfigWidget.hpp"

DepthCalcConfigWidget::DepthCalcConfigWidget(QWidget *parent)
    : QWidget(parent) {
  auto label1 = new QLabel("Fmod");
  lineEditFmod_ = new QLineEdit(this);
  QHBoxLayout *layout1 = new QHBoxLayout();
  layout1->addWidget(label1);
  layout1->addWidget(lineEditFmod_);
  layout1->addWidget(new QLabel("MHz"));

  auto label2 = new QLabel("Adjust");
  lineEditOffset_ = new QLineEdit(this);
  QHBoxLayout *layout2 = new QHBoxLayout();
  layout2->addWidget(label2);
  layout2->addWidget(lineEditOffset_);
  layout2->addWidget(new QLabel("m"));

  lineEditFmod_->setText("0");
  lineEditOffset_->setText("0");
  lineEditFmod_->setMaximumSize(30, 50);
  lineEditOffset_->setMaximumSize(30, 50);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(layout1);
  mainLayout->addLayout(layout2);
  setLayout(mainLayout);

  this->setMaximumWidth(150);

  connect(lineEditFmod_, &QLineEdit::textChanged, this,
          &DepthCalcConfigWidget::onConfigChanged);
  connect(lineEditOffset_, &QLineEdit::textChanged, this,
          &DepthCalcConfigWidget::onConfigChanged);
};

void DepthCalcConfigWidget::SetTransformer(DepthCalc *transformer) {
  transformer_ = transformer;
}

void DepthCalcConfigWidget::onConfigChanged(const QString &text) {
  float fmod = lineEditFmod_->text().toFloat();
  float offset = lineEditOffset_->text().toFloat();

  transformer_->SetConfig(fmod, offset);
}
