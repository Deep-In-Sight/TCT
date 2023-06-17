#include "PlaybackSettingWidget.hpp"

PlaybackSettingWidget::PlaybackSettingWidget(QWidget *parent)
    : QWidget(parent) {
  // create a row consist of the linedit and a label on the left
  auto *layout1 = new QHBoxLayout();
  auto *label1 = new QLabel("Source", this);
  _lineEditFilePath = new QLineEdit(this);
  layout1->addWidget(label1);
  layout1->addWidget(_lineEditFilePath);

  // create a row consist of the spinbox and a label on the left
  auto *layout2 = new QHBoxLayout();
  auto *label2 = new QLabel("FPS", this);
  _spinBoxFps = new QDoubleSpinBox(this);
  _spinBoxFps->setRange(0.0, 1000.0);
  _spinBoxFps->setSingleStep(0.1);
  _spinBoxFps->setValue(30.0);
  _spinBoxFps->setMaximumSize(100, 30);
  layout2->addWidget(label2);
  layout2->addWidget(_spinBoxFps);

  // a row with a checkbox to enable loop
  auto *layout3 = new QHBoxLayout();
  auto *label3 = new QLabel("Loop", this);
  _checkboxLoop = new QCheckBox(this);
  _checkboxLoop->setCheckState(Qt::Unchecked);
  layout3->addWidget(label3);
  layout3->addWidget(_checkboxLoop);

  auto *layout4 = new QHBoxLayout();
  auto *label5 = new QLabel("Width", this);
  _lineEditWidth = new QLineEdit(this);
  _lineEditWidth->setText("640");
  auto *label6 = new QLabel("Height", this);
  _lineEditHeight = new QLineEdit(this);
  _lineEditHeight->setText("480");
  layout4->addWidget(label5);
  layout4->addWidget(_lineEditWidth);
  layout4->addWidget(label6);
  layout4->addWidget(_lineEditHeight);
  _lineEditWidth->setMaximumSize(50, 30);
  _lineEditHeight->setMaximumSize(50, 30);

  // create a button to start/stop the playback
  auto *layout5 = new QHBoxLayout();
  _buttonPlayStop = new QPushButton("Play", this);
  layout5->addWidget(_buttonPlayStop);

  // create a vertical layout to hold the two rows
  auto *layout = new QVBoxLayout(this);
  layout->addLayout(layout1);
  layout->addLayout(layout2);
  layout->addLayout(layout3);
  layout->addLayout(layout4);
  layout->addLayout(layout5);

  // set the layout to the widget
  setLayout(layout);

  // connect the signal and slot
  connect(_lineEditFilePath, &QLineEdit::textEdited, this,
          &PlaybackSettingWidget::onFilenameChanged);
  connect(_spinBoxFps, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
          &PlaybackSettingWidget::onFpsChanged);
  connect(_checkboxLoop, &QCheckBox::stateChanged, this,
          &PlaybackSettingWidget::onLoopChanged);
  connect(_buttonPlayStop, &QPushButton::clicked, this,
          &PlaybackSettingWidget::onPlayStopClicked);
  connect(_lineEditWidth, &QLineEdit::textEdited, this,
          &PlaybackSettingWidget::onShapeChanged);
  connect(_lineEditHeight, &QLineEdit::textEdited, this,
          &PlaybackSettingWidget::onShapeChanged);
};

PlaybackSettingWidget::~PlaybackSettingWidget(){};

void PlaybackSettingWidget::setPlaybackSource(PlaybackSource *source) {
  _source = source;
}
QString PlaybackSettingWidget::getFilename() const { return _filename; }
double PlaybackSettingWidget::getFps() const { return _fps; }
void PlaybackSettingWidget::setFilename(QString const &string) {
  _lineEditFilePath->setText(string);
}
void PlaybackSettingWidget::setFps(double value) {
  _spinBoxFps->setValue(value);
}

void PlaybackSettingWidget::onFilenameChanged(QString const &string) {
  if (_source) {
    _source->SetFilename(string.toStdString());
  }
  _filename = string;
};

void PlaybackSettingWidget::onFpsChanged(double value) {
  if (_source) {
    _source->SetFrameRate(value);
  }
  _fps = value;
};

void PlaybackSettingWidget::onLoopChanged(int state) {
  if (_source) {
    _source->SetLoop(state == Qt::Checked);
  }
};

void PlaybackSettingWidget::onPlayStopClicked() {
  if (_source) {
    if (_source->GetState() == kStreamStatePlaying) {
      _source->Stop();
      _buttonPlayStop->setText("Play");
    } else if (_source->GetState() == kStreamStateStopped) {
      _source->Start();
      _buttonPlayStop->setText("Stop");
    }
  }
};

void PlaybackSettingWidget::onShapeChanged(QString const &string) {
  if (_source) {
    int width = _lineEditWidth->text().toInt();
    int height = _lineEditHeight->text().toInt();
    _source->SetFormat({4, height, width}, CV_16SC1);
  }
};