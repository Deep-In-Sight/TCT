#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/base-sink.h>
#include <sdk/tof/playback-src.h>

class SinkMock : public BaseSink {
 public:
  SinkMock(const string& name) : BaseSink(name) {}
  ~SinkMock() {}

  MOCK_METHOD(void, SinkFrame, (cv::Mat & frame), (override));
};

TEST(PlaybackSource, InitializeSource) {
  PlaybackSource source("playback", false, false);
  source.SetFilename("data/input/37MHz_quadphase_4x4_2fps_1s.bin");
  EXPECT_TRUE(source.InitializeSource());
  source.SetFilename("data/input/doesnt_exist.bin");
  EXPECT_FALSE(source.InitializeSource());
}

TEST(PlaybackSource, GenerateFrame) {
  SinkMock sink("sink");
  PlaybackSource source("playback", false, false);
  source.GetSourcePad()->Link(sink.GetSinkPad());
  source.SetFilename("data/input/37MHz_quadphase_4x4_2fps_1s.bin");
  source.SetFormat({4, 4, 4}, CV_16SC1);

  EXPECT_CALL(sink, SinkFrame).Times(2);
  MatShape shape;
  int type;
  sink.GetSinkPad()->GetFrameFormat(shape, type);
  EXPECT_EQ(shape.dims(), 3);
  EXPECT_EQ(shape[0], 4);
  EXPECT_EQ(shape[1], 4);
  EXPECT_EQ(shape[2], 4);
  EXPECT_EQ(type, CV_16SC1);

  source.Start();
  this_thread::sleep_for(chrono::milliseconds(10));
  source.Stop();
}