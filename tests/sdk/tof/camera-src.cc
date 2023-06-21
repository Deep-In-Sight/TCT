#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/tof/camera-src.h>

class ToFCameraTest : public ::testing::Test {
 protected:
  void SetUp() override {
    string deviceName_ = "192.168.7.2:50660";
    tofCamera_ = new ToFCameraSrc(deviceName_);
  }
  void TearDown() override { delete tofCamera_; }
  ToFCameraSrc *tofCamera_;
};

TEST_F(ToFCameraTest, TestInitialize) {
  EXPECT_NO_THROW(tofCamera_->InitializeSource());
}

TEST_F(ToFCameraTest, TestGenerateFrame) {
  tofCamera_->InitializeSource();
  Mat m = tofCamera_->GenerateFrame();
  EXPECT_EQ(m.size[0], 4);
  EXPECT_EQ(m.size[1], 480);
  EXPECT_EQ(m.size[2], 640);
  EXPECT_EQ(m.type(), CV_16SC1);
}
