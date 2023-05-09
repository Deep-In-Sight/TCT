#include <gmock/gmock.h>
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <lib/inspector/inspector-tracker.h>

#include <iostream>

class InspectorTrackerMock : public InspectorTracker {
 public:
  virtual ~InspectorTrackerMock(){};
  MOCK_METHOD(void, RenderPoint, (float value), (override));

  float GetPoint(GstBuffer* buffer) {
    // call the base class implementation
    return InspectorTracker::GetPoint(buffer);
  }
};

class InspectorTrackerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    inspector_tracker_mock_ = new InspectorTrackerMock();
    inspector_tracker_mock_->SetFrameSize(test_width_, test_height_);
  }

  void TearDown() override { delete inspector_tracker_mock_; }

  int test_width_ = 10;
  int test_height_ = 10;
  InspectorTrackerMock* inspector_tracker_mock_;
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

TEST_F(InspectorTrackerTest, TestGetPoint) {
  float* data = new float[100];
  for (int i = 0; i < 100; i++) {
    data[i] = i;
  }
  GstBuffer* buffer = gst_buffer_new_wrapped(data, sizeof(data));
  inspector_tracker_mock_->SetLocation(1, 2);
  float value = inspector_tracker_mock_->GetPoint(buffer);
  EXPECT_EQ(value, 21);

  gst_buffer_unref(buffer);
}