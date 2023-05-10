#include <gmock/gmock.h>
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <sdk/inspector/inspector-histogram.h>

#include <iostream>
#include <limits>

class InspectorHistogramMock : public InspectorHistogram {
 public:
  InspectorHistogramMock(){};
  ~InspectorHistogramMock(){};
  /* mock-away so the abstract InspectorHistogram can be instantiated. */
  MOCK_METHOD(void, RenderHistogram, (const HistogramData& histogram),
              (override));
  const HistogramData& GetHistogram(GstBuffer* buffer) {
    const HistogramData& hist = InspectorHistogram::GetHistogram(buffer);
    return hist;
  }
};

class InspectorHistogramTest : public ::testing::Test {
 protected:
  void SetUp() override {
    inspector_histogram_mock_ = new InspectorHistogramMock();
    inspector_histogram_mock_->SetFrameSize(test_width_, test_height_);
  }

  void TearDown() override { delete inspector_histogram_mock_; }

  InspectorHistogramMock* inspector_histogram_mock_;
  int test_width_ = 10;
  int test_height_ = 10;
};

TEST_F(InspectorHistogramTest, TestSetRange) {
  int x, y, x2, y2;
  // in-bound range
  inspector_histogram_mock_->SetRange(1, 2, 3, 4);
  inspector_histogram_mock_->GetRange(x, y, x2, y2);
  EXPECT_EQ(x, 1);
  EXPECT_EQ(y, 2);
  EXPECT_EQ(x2, 3);
  EXPECT_EQ(y2, 4);

  // out-bound range
  inspector_histogram_mock_->SetRange(-1, -2, 15, 20);
  inspector_histogram_mock_->GetRange(x, y, x2, y2);
  EXPECT_EQ(x, 0);
  EXPECT_EQ(y, 0);
  EXPECT_EQ(x2, 9);
  EXPECT_EQ(y2, 9);

  // reversed range
  inspector_histogram_mock_->SetRange(3, 4, 1, 2);
  inspector_histogram_mock_->GetRange(x, y, x2, y2);
  EXPECT_EQ(x, 1);
  EXPECT_EQ(y, 2);
  EXPECT_EQ(x2, 3);
  EXPECT_EQ(y2, 4);
}

TEST_F(InspectorHistogramTest, TestSetBinsNonUniform) {
  std::vector<float> edges = {1, 3, 2, 4};
  inspector_histogram_mock_->SetBins(edges);
  auto actual_edges = inspector_histogram_mock_->GetEdges();
  EXPECT_EQ(actual_edges.size(), 6);
  EXPECT_EQ(actual_edges[0], std::numeric_limits<float>::lowest());
  EXPECT_EQ(actual_edges[1], 1);
  EXPECT_EQ(actual_edges[2], 2);
  EXPECT_EQ(actual_edges[3], 3);
  EXPECT_EQ(actual_edges[4], 4);
  EXPECT_EQ(actual_edges[5], std::numeric_limits<float>::max());
}

TEST_F(InspectorHistogramTest, TestSetBinsUniform) {
  float min = 0;
  float max = 10;
  int num_bins = 5;
  inspector_histogram_mock_->SetBins(min, max, num_bins);
  auto edges = inspector_histogram_mock_->GetEdges();
  EXPECT_EQ(edges.size(), 8);
  EXPECT_EQ(edges[0], std::numeric_limits<float>::lowest());
  EXPECT_EQ(edges[1], 0);
  EXPECT_EQ(edges[2], 2);
  EXPECT_EQ(edges[3], 4);
  EXPECT_EQ(edges[4], 6);
  EXPECT_EQ(edges[5], 8);
  EXPECT_EQ(edges[6], 10);
  EXPECT_EQ(edges[7], std::numeric_limits<float>::max());
}

TEST_F(InspectorHistogramTest, TestGetHistogram) {
  int bins = 2;
  float min = 0, max = 10;
  inspector_histogram_mock_->SetBins(min, max, bins);
  float* data = new float[100];
  for (int i = 0; i < 100; i++) {
    data[i] = i;
  }
  GstBuffer* buffer = gst_buffer_new_wrapped(data, sizeof(data));

  inspector_histogram_mock_->SetRange(0, 0, 10, 10);

  const HistogramData& hist = inspector_histogram_mock_->GetHistogram(buffer);
  EXPECT_EQ(hist.counts.size(), 4);
  EXPECT_EQ(hist.counts[0], 0);
  EXPECT_EQ(hist.counts[1], 5);
  EXPECT_EQ(hist.counts[2], 5);
  EXPECT_EQ(hist.counts[3], 90);
  EXPECT_EQ(hist.edges.size(), 5);
  EXPECT_EQ(hist.edges[0], std::numeric_limits<float>::lowest());
  EXPECT_EQ(hist.edges[1], 0);
  EXPECT_EQ(hist.edges[2], 5);
  EXPECT_EQ(hist.edges[3], 10);
  EXPECT_EQ(hist.edges[4], std::numeric_limits<float>::max());

  std::vector<float> edges = {5, 10, 15};
  inspector_histogram_mock_->SetBins(edges);
  const HistogramData& hist2 = inspector_histogram_mock_->GetHistogram(buffer);
  EXPECT_EQ(hist2.counts.size(), 4);
  EXPECT_EQ(hist2.counts[0], 5);
  EXPECT_EQ(hist2.counts[1], 5);
  EXPECT_EQ(hist2.counts[2], 5);
  EXPECT_EQ(hist2.counts[3], 85);
  EXPECT_EQ(hist2.edges.size(), 5);
  EXPECT_EQ(hist2.edges[0], std::numeric_limits<float>::lowest());
  EXPECT_EQ(hist2.edges[1], 5);
  EXPECT_EQ(hist2.edges[2], 10);
  EXPECT_EQ(hist2.edges[3], 15);
  EXPECT_EQ(hist2.edges[4], std::numeric_limits<float>::max());

  gst_buffer_unref(buffer);
}