#include <gmock/gmock.h>
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <sdk/inspector/inspector-tracker.h>

#include <iostream>

using testing::NiceMock;
class InspectorTrackerMock : public InspectorTracker {
 public:
  virtual ~InspectorTrackerMock(){};
  MOCK_METHOD(void, RenderPoint, (float value), (override));
  MOCK_METHOD(void, OnFrameFormatChanged, (const MatShape& shape, int type),
              (override));

  float GetPoint(Mat& frame) {
    // call the base class implementation
    return InspectorTracker::GetPoint(frame);
  }
};

class InspectorTrackerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    inspector_tracker_mock_ = new NiceMock<InspectorTrackerMock>();
    inspector_tracker_mock_->SetFrameFormat({2, 10, 10}, CV_32FC1);
  }

  void TearDown() override { delete inspector_tracker_mock_; }

  int test_width_ = 10;
  int test_height_ = 10;
  NiceMock<InspectorTrackerMock>* inspector_tracker_mock_;
};

TEST_F(InspectorTrackerTest, TestSetLocation) {
  int x, y;

  // in-bound location
  inspector_tracker_mock_->SetLocation(1, 2);
  inspector_tracker_mock_->GetLocation(x, y);
  EXPECT_EQ(x, 1);
  EXPECT_EQ(y, 2);

  // out-bound location
  inspector_tracker_mock_->SetLocation(-1, 15);
  inspector_tracker_mock_->GetLocation(x, y);
  EXPECT_EQ(x, 0);
  EXPECT_EQ(y, 9);

  inspector_tracker_mock_->SetLocation(-1, -1);
  inspector_tracker_mock_->GetLocation(x, y);
  EXPECT_EQ(x, 0);
  EXPECT_EQ(y, 0);

  inspector_tracker_mock_->SetLocation(15, 15);
  inspector_tracker_mock_->GetLocation(x, y);
  EXPECT_EQ(x, 9);
  EXPECT_EQ(y, 9);
}

TEST_F(InspectorTrackerTest, TestGetDepthPoint) {
  float* data = new float[200];
  for (int i = 0; i < 200; i++) {
    data[i] = i;
  }
  Mat frame({2, 10, 10}, CV_32FC1, data);
  inspector_tracker_mock_->SetLocation(1, 2);
  inspector_tracker_mock_->SelectChannel(kDepthChannel);
  float value = inspector_tracker_mock_->GetPoint(frame);
  EXPECT_EQ(value, 21);
}

TEST_F(InspectorTrackerTest, TestGetAmplitudePoint) {
  float* data = new float[200];
  for (int i = 0; i < 200; i++) {
    data[i] = i;
  }
  Mat frame({2, 10, 10}, CV_32FC1, data);
  inspector_tracker_mock_->SetLocation(1, 2);
  inspector_tracker_mock_->SelectChannel(kAmplitudeChannel);
  float value = inspector_tracker_mock_->GetPoint(frame);
  EXPECT_EQ(value, 121);
}

TEST_F(InspectorTrackerTest, TestGetDepthPointOneChannel) {
  inspector_tracker_mock_->SetFrameFormat({1, 10, 10}, CV_32FC1);
  float* data = new float[200];
  for (int i = 0; i < 200; i++) {
    data[i] = i;
  }
  Mat frame({1, 10, 10}, CV_32FC1, data);
  inspector_tracker_mock_->SetLocation(1, 2);
  inspector_tracker_mock_->SelectChannel(kDepthChannel);
  float value = inspector_tracker_mock_->GetPoint(frame);
  EXPECT_EQ(value, 21);
}

TEST_F(InspectorTrackerTest, TestGetAmplitudePointOneChannel) {
  inspector_tracker_mock_->SetFrameFormat({1, 10, 10}, CV_32FC1);
  float* data = new float[200];
  for (int i = 0; i < 200; i++) {
    data[i] = i;
  }
  Mat frame({1, 10, 10}, CV_32FC1, data);
  inspector_tracker_mock_->SetLocation(1, 2);
  EXPECT_ANY_THROW({
    inspector_tracker_mock_->SelectChannel(kAmplitudeChannel);
    float value = inspector_tracker_mock_->GetPoint(frame);
  });
}