#include <gmock/gmock.h>
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <sdk/inspector/inspector-client.h>

#include <iostream>

class InspectorClientMock : public InspectorClient {
 public:
  virtual ~InspectorClientMock(){};
  MOCK_METHOD(void, Update, (GstBuffer*), (override));
};

class InspectorClientTest : public ::testing::Test {
 protected:
  void SetUp() override { test_client = new InspectorClientMock(); }

  void TearDown() override { delete test_client; }

  InspectorClientMock* test_client;
};

TEST_F(InspectorClientTest, TestSetCaps) {
  char caps_str[128];
  int frame_width = 640;
  int frame_height = 480;
  sprintf(caps_str, "video/tof,format=DA_F32,width=%d,height=%d", frame_width,
          frame_height);
  GstCaps* caps = gst_caps_from_string(caps_str);
  test_client->SetCaps(caps);
  gst_caps_unref(caps);

  int width, height;
  test_client->GetFrameSize(width, height);

  EXPECT_EQ(width, frame_width);
  EXPECT_EQ(height, frame_height);
}