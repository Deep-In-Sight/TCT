#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/base-sink.h>
#include <sdk/tof/depth-calc.h>
#include <sdk/tof/playback-src.h>

using ::testing::NiceMock;

class TestSink : public BaseSink {
 public:
  TestSink(const string& name) : BaseSink(name) {}
  ~TestSink() {}

  void SinkFrame(Mat& frame) override { frame_ = frame; }
  Mat frame_;
};

class DepthCalcTest : public ::testing::Test {
 protected:
  void SetUp() override {
    source_ = new PlaybackSource("playback", false, false);
    depth_calc_ = new DepthCalc("depth_calc");
    sink_ = new TestSink("sink");
    source_->GetSourcePad()->Link(depth_calc_->GetSinkPad());
    depth_calc_->GetSourcePad()->Link(sink_->GetSinkPad());
    source_->SetFilename("data/input/37MHz_quadphase_8x8.bin");
    source_->SetFormat({4, 8, 8}, CV_16SC1);
    // really need some way to encode these parameters somewhere.. test config
    // file? test data filename?
    depth_calc_->SetConfig(37e6, 1.4);
    expected_ = Mat({2, 8, 8}, CV_32FC1);
    FILE* fp = fopen("data/golden_output/37MHz_depth_amplitude_8x8.bin", "rb");
    assert(fp != nullptr);
    int read = fread(expected_.data, sizeof(float), 8 * 8 * 2, fp);
    assert(read == 2 * 8 * 8);
    fclose(fp);
  }

  void TearDown() override {
    delete sink_;
    delete source_;
    delete depth_calc_;
  }

  PlaybackSource* source_;
  DepthCalc* depth_calc_;
  TestSink* sink_;
  Mat expected_;
};

TEST_F(DepthCalcTest, TestFormatChanged) {
  MatShape shape;
  int type;
  depth_calc_->GetSinkPad()->GetFrameFormat(shape, type);
  EXPECT_EQ(shape.dims(), 3);
  EXPECT_EQ(shape[0], 4);
  EXPECT_EQ(shape[1], 8);
  EXPECT_EQ(shape[2], 8);
  EXPECT_EQ(type, CV_16SC1);
  depth_calc_->GetSourcePad()->GetFrameFormat(shape, type);
  EXPECT_EQ(shape.dims(), 3);
  EXPECT_EQ(shape[0], 2);
  EXPECT_EQ(shape[1], 8);
  EXPECT_EQ(shape[2], 8);
  EXPECT_EQ(type, CV_32FC1);
  sink_->GetSinkPad()->GetFrameFormat(shape, type);
  EXPECT_EQ(shape.dims(), 3);
  EXPECT_EQ(shape[0], 2);
  EXPECT_EQ(shape[1], 8);
  EXPECT_EQ(shape[2], 8);
  EXPECT_EQ(type, CV_32FC1);
}

TEST_F(DepthCalcTest, DepthTest) {
  source_->Start();
  this_thread::sleep_for(chrono::milliseconds(10));
  source_->Stop();

  Mat actual = sink_->frame_;

  EXPECT_EQ(actual.size[0], expected_.size[0]);
  EXPECT_EQ(actual.size[1], expected_.size[1]);
  EXPECT_EQ(actual.size[2], expected_.size[2]);
  for (int y = 0; y < actual.size[1]; y++) {
    for (int x = 0; x < actual.size[2]; x++) {
      EXPECT_NEAR(actual.at<float>(0, y, x), expected_.at<float>(0, y, x),
                  1e-3);
      EXPECT_NEAR(actual.at<float>(1, y, x), expected_.at<float>(1, y, x),
                  1e-3);
    }
  }
}