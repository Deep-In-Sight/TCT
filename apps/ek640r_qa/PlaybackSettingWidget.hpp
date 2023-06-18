#ifndef __PLAYBACK_SOURCE_SETTING_WIDGET_H__
#define __PLAYBACK_SOURCE_SETTING_WIDGET_H__

#include <sdk/tof/playback-src.h>

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class PlaybackSettingWidget : public QWidget {
  Q_OBJECT

 public:
  PlaybackSettingWidget(QWidget *parent = nullptr);

  ~PlaybackSettingWidget();

 public:
  void setPlaybackSource(PlaybackSource *source);
  QString getFilename() const;
  double getFps() const;
  void setFilename(QString const &string);
  void setFps(double value);

 private Q_SLOTS:

  void onFilenameChanged(QString const &string);
  void onFpsChanged(double value);
  void onLoopChanged(int state);
  void onShapeChanged(QString const &string);
  void onPlayStopClicked();

 private:
  PlaybackSource *_source;
  QString _filename;
  double _fps;

  QLineEdit *_lineEditFilePath;
  QDoubleSpinBox *_spinBoxFps;
  QLineEdit *_lineEditWidth;
  QLineEdit *_lineEditHeight;
  QPushButton *_buttonPlayStop;
  QCheckBox *_checkboxLoop;
};

#endif  // __PLAYBACK_SOURCE_SETTING_WIDGET_H__