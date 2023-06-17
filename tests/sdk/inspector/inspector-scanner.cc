#include <gmock/gmock.h>
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <sdk/inspector/inspector-scanner.h>

#include <iostream>

using testing::NiceMock;
class InspectorHScannerMock : public InspectorHScanner {
 public:
  MOCK_METHOD(void, RenderRange, (const std::vector<float>& vec), (override));
  MOCK_METHOD(void, OnFrameFormatChanged, (const MatShape& shape, int type),
              (override));
  const vector<float>& CollectRange(Mat& frame) {
    return InspectorHScanner::CollectRange(frame);
  }
};

class InspectorVScannerMock : public InspectorVScanner {
 public:
  MOCK_METHOD(void, RenderRange, (const std::vector<float>& vec), (override));
  MOCK_METHOD(void, OnFrameFormatChanged, (const MatShape& shape, int type),
              (override));
  const vector<float>& CollectRange(Mat& frame) {
    return InspectorVScanner::CollectRange(frame);
  }
};

class InspectorScannerTest : public ::testing::Test {
 public:
  InspectorScannerTest() {
    hscanner_ = new NiceMock<InspectorHScannerMock>();
    vscanner_ = new NiceMock<InspectorVScannerMock>();
    data_ = new float[200];
    for (int i = 0; i < 200; i++) {
      data_[i] = i;
    }
  }
  ~InspectorScannerTest() {
    delete hscanner_;
    delete vscanner_;
    delete data_;
  }
  int test_width_ = 10;
  int test_height_ = 10;
  float* data_;
  NiceMock<InspectorHScannerMock>* hscanner_;
  NiceMock<InspectorVScannerMock>* vscanner_;
};

TEST_F(InspectorScannerTest, TestSetHRange) {
  int x1, y1, xy2;
  hscanner_->SetFrameFormat({2, 10, 10}, CV_32FC1);
  // in-bound range
  hscanner_->SetRoi(1, 2, 3);
  hscanner_->GetRoi(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 3);

  // out-bound range
  hscanner_->SetRoi(-1, 2, 15);
  hscanner_->GetRoi(x1, y1, xy2);
  EXPECT_EQ(x1, 0);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 9);

  // reverse selection
  hscanner_->SetRoi(3, 2, 1);
  hscanner_->GetRoi(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 3);
}

TEST_F(InspectorScannerTest, TestSetVRange) {
  int x1, y1, xy2;
  vscanner_->SetFrameFormat({2, 10, 10}, CV_32FC1);
  // in-bound range
  vscanner_->SetRoi(1, 2, 3);
  vscanner_->GetRoi(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 3);

  // out-bound range
  vscanner_->SetRoi(1, -2, 15);
  vscanner_->GetRoi(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 0);
  EXPECT_EQ(xy2, 9);

  // reverse selection
  vscanner_->SetRoi(1, 3, 2);
  vscanner_->GetRoi(x1, y1, xy2);
  EXPECT_EQ(x1, 1);
  EXPECT_EQ(y1, 2);
  EXPECT_EQ(xy2, 3);
}

TEST_F(InspectorScannerTest, TestCollectRangeOneChannel) {
  Mat frame({1, 10, 10}, CV_32FC1, data_);

  hscanner_->SelectChannel(kDepthChannel);
  hscanner_->SetRoi(1, 2, 3);
  auto& hvec = hscanner_->CollectRange(frame);

  EXPECT_EQ(hvec.size(), 3);
  EXPECT_EQ(hvec[0], 21);
  EXPECT_EQ(hvec[1], 22);
  EXPECT_EQ(hvec[2], 23);

  // vertical scan
  vscanner_->SelectChannel(kDepthChannel);
  vscanner_->SetRoi(1, 2, 3);
  auto& vvec = vscanner_->CollectRange(frame);
  EXPECT_EQ(vvec.size(), 2);
  EXPECT_EQ(vvec[0], 21);
  EXPECT_EQ(vvec[1], 31);

  EXPECT_ANY_THROW({
    hscanner_->SetFrameFormat({1, 10, 10}, CV_32FC1);
    hscanner_->SelectChannel(kAmplitudeChannel);
  });
}

TEST_F(InspectorScannerTest, TestCollectRangeAmplitude) {
  Mat frame({2, 10, 10}, CV_32FC1, data_);
  hscanner_->SetFrameFormat({2, 10, 10}, CV_32FC1);

  hscanner_->SelectChannel(kDepthChannel);
  hscanner_->SetRoi(1, 2, 3);
  auto& hvec = hscanner_->CollectRange(frame);

  EXPECT_EQ(hvec.size(), 3);
  EXPECT_EQ(hvec[0], 21);
  EXPECT_EQ(hvec[1], 22);
  EXPECT_EQ(hvec[2], 23);

  vscanner_->SetFrameFormat({2, 10, 10}, CV_32FC1);
  vscanner_->SelectChannel(kAmplitudeChannel);
  vscanner_->SetRoi(1, 2, 3);
  auto& vvec = vscanner_->CollectRange(frame);
  EXPECT_EQ(vvec.size(), 2);
  EXPECT_EQ(vvec[0], 121);
  EXPECT_EQ(vvec[1], 131);
}