#include <gmock/gmock.h>
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <sdk/inspector/inspector-histogram.h>

#include <iostream>
#include <limits>

using testing::NiceMock;

class InspectorHistogramMock : public InspectorHistogram {
 public:
  InspectorHistogramMock(){};
  ~InspectorHistogramMock(){};
  /* mock-away so the abstract InspectorHistogram can be instantiated. */
  MOCK_METHOD(void, RenderHistogram, (const Mat& histogram), (override));
  MOCK_METHOD(void, OnFrameFormatChanged, (const MatShape& shape, int type),
              (override));
  const Mat& CalculateHistogram(Mat& frame) {
    auto& hist = InspectorHistogram::CalculateHistogram(frame);
    return hist;
  }
};

class InspectorHistogramTest : public ::testing::Test {
 protected:
  void SetUp() override {
    inspector_histogram_mock_ = new NiceMock<InspectorHistogramMock>();
    data_ = new float[100];
    for (int i = 0; i < 100; i++) {
      data_[i] = i;
    }
  }

  void TearDown() override {
    delete inspector_histogram_mock_;
    delete data_;
  }

  NiceMock<InspectorHistogramMock>* inspector_histogram_mock_;
  float* data_;
  int test_width_ = 10;
  int test_height_ = 10;
};

TEST_F(InspectorHistogramTest, TestSetRange) {
  int x, y, x2, y2;
  inspector_histogram_mock_->SetFrameFormat({1, 10, 10}, CV_32FC1);
  // in-bound range
  inspector_histogram_mock_->SetRoi(1, 2, 3, 4);
  inspector_histogram_mock_->GetRoi(x, y, x2, y2);
  EXPECT_EQ(x, 1);
  EXPECT_EQ(y, 2);
  EXPECT_EQ(x2, 3);
  EXPECT_EQ(y2, 4);

  // out-bound range
  inspector_histogram_mock_->SetRoi(-1, -2, 15, 20);
  inspector_histogram_mock_->GetRoi(x, y, x2, y2);
  EXPECT_EQ(x, 0);
  EXPECT_EQ(y, 0);
  EXPECT_EQ(x2, 9);
  EXPECT_EQ(y2, 9);

  // reversed range
  inspector_histogram_mock_->SetRoi(3, 4, 1, 2);
  inspector_histogram_mock_->GetRoi(x, y, x2, y2);
  EXPECT_EQ(x, 1);
  EXPECT_EQ(y, 2);
  EXPECT_EQ(x2, 3);
  EXPECT_EQ(y2, 4);
}

TEST_F(InspectorHistogramTest, TestSetBinsUniform) {
  float min = 0;
  float max = 10;
  int num_bins = 5;
  inspector_histogram_mock_->SetBins(min, max, num_bins);
  auto edges = inspector_histogram_mock_->GetEdges();
  EXPECT_EQ(edges.size(), 6);
  EXPECT_EQ(edges[0], 0);
  EXPECT_EQ(edges[1], 2);
  EXPECT_EQ(edges[2], 4);
  EXPECT_EQ(edges[3], 6);
  EXPECT_EQ(edges[4], 8);
  EXPECT_EQ(edges[5], 10);
}

TEST_F(InspectorHistogramTest, TestGetHistogram) {
  int bins = 2;
  float min = 0, max = 10;
  inspector_histogram_mock_->SetFrameFormat({1, 10, 10}, CV_32FC1);
  inspector_histogram_mock_->SelectChannel(kDepthChannel);
  inspector_histogram_mock_->SetBins(min, max, bins);

  inspector_histogram_mock_->SetRoi(0, 0, 10, 10);

  Mat frame({1, 10, 10}, CV_32FC1, data_);

  const Mat& hist = inspector_histogram_mock_->CalculateHistogram(frame);

  EXPECT_EQ(hist.total(), 2);
  EXPECT_EQ(hist.at<float>(0), 5);
  EXPECT_EQ(hist.at<float>(1), 5);
}