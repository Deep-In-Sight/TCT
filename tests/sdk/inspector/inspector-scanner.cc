#include <gmock/gmock.h>
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <sdk/inspector/inspector-scanner.h>

#include <iostream>

class InspectorScannerMock : public InspectorScanner {
 public:
  virtual ~InspectorScannerMock(){};
  /* mock-away so the abstract InspectorScanner can be instantiated. */
  MOCK_METHOD(void, SetRange, (int x1, int y1, int xy2), (override));
  MOCK_METHOD(const std::vector<float>&, CollectRange, (GstBuffer*),
              (override));
  MOCK_METHOD(void, RenderResult, (const std::vector<float>& vec), (override));
  void SetHRange(int x1, int y1, int xy2) {
    SetRangeImpl(x1, y1, xy2, InspectorScanner::kScanHorizontal);
  }
  void SetVRange(int x1, int y1, int xy2) {
    SetRangeImpl(x1, y1, xy2, InspectorScanner::kScanVertical);
  }
  const std::vector<float>& CollectHRange(GstBuffer* buffer) {
    return CollectRangeImpl(buffer, InspectorScanner::kScanHorizontal);
  }
  const std::vector<float>& CollectVRange(GstBuffer* buffer) {
    return CollectRangeImpl(buffer, InspectorScanner::kScanVertical);
  }
};

class InspectorScannerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    inspector_scanner_mock_ = new InspectorScannerMock();
    inspector_scanner_mock_->SetFrameSize(test_width_, test_height_);
  }

  void TearDown() override { delete inspector_scanner_mock_; }

  int test_width_ = 10;
  int test_height_ = 10;
  InspectorScannerMock* inspector_scanner_mock_;
};

TEST_F(InspectorScannerTest, TestSetRange) {
  int x1, y1, xy2;
  // in-bound range
  inspector_scanner_mock_->SetHRange(1, 2, 3);
  inspector_scanner_mock_->GetRange(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 3);

  inspector_scanner_mock_->SetVRange(1, 2, 3);
  inspector_scanner_mock_->GetRange(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 3);

  // out-bound range
  inspector_scanner_mock_->SetHRange(-1, 2, 15);
  inspector_scanner_mock_->GetRange(x1, y1, xy2);
  EXPECT_EQ(x1, 0);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 9);

  inspector_scanner_mock_->SetVRange(1, -2, 15);
  inspector_scanner_mock_->GetRange(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 0);
  EXPECT_EQ(xy2, 9);

  // reverse selection
  inspector_scanner_mock_->SetHRange(3, 2, 1);
  inspector_scanner_mock_->GetRange(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 3);

  inspector_scanner_mock_->SetVRange(1, 3, 2);
  inspector_scanner_mock_->GetRange(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 3);
}

TEST_F(InspectorScannerTest, TestCollectRange) {
  std::vector<float> vec;
  float* data = new float[100];
  for (int i = 0; i < 100; i++) {
    data[i] = i;
  }
  GstBuffer* buffer = gst_buffer_new_wrapped(data, sizeof(data));

  // horizontal scan
  inspector_scanner_mock_->SetHRange(1, 2, 3);
  vec = inspector_scanner_mock_->CollectHRange(buffer);

  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 21);
  EXPECT_EQ(vec[1], 22);
  EXPECT_EQ(vec[2], 23);

  // vertical scan
  inspector_scanner_mock_->SetVRange(1, 2, 3);
  vec = inspector_scanner_mock_->CollectVRange(buffer);
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], 21);
  EXPECT_EQ(vec[1], 31);

  gst_buffer_unref(buffer);
}