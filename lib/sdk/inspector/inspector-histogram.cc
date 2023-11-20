/* Copyright (C) 2023 Deep In Sight
 * Author: Le Ngoc Linh <lnlinh93@dinsight.ai>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */

#include <sdk/inspector/inspector-histogram.h>

#include <algorithm>
#include <cmath>
#include <limits>

InspectorHistogram::InspectorHistogram() {
  roi_ = Rect(0, 0, mat_shape_[2], mat_shape_[1]);
  ranges_[0] = 0;
  ranges_[1] = 0;
  bins_ = 50;
  isAutoRange_ = true;
}

void InspectorHistogram::SetRoi(int x, int y, int x2, int y2) {
  int tl_x = max(0, min(x, mat_shape_[2] - 1));
  int tl_y = max(0, min(y, mat_shape_[1] - 1));
  int br_x = max(0, min(x2, mat_shape_[2] - 1));
  int br_y = max(0, min(y2, mat_shape_[1] - 1));
  if (tl_x > br_x) {
    swap(tl_x, br_x);
  }
  if (tl_y > br_y) {
    swap(tl_y, br_y);
  }
  roi_.x = tl_x;
  roi_.y = tl_y;
  roi_.width = br_x - tl_x + 1;
  roi_.height = br_y - tl_y + 1;
}

void InspectorHistogram::GetRoi(int& x, int& y, int& x2, int& y2) {
  x = roi_.x;
  y = roi_.y;
  x2 = roi_.x + roi_.width - 1;
  y2 = roi_.y + roi_.height - 1;
}

void InspectorHistogram::SetBins(int num_bins) { bins_ = num_bins; }

void InspectorHistogram::SetRanges(float min, float max) {
  ranges_[0] = min;
  ranges_[1] = max;
  isAutoRange_ = false;
}

std::vector<float> InspectorHistogram::GetEdges() {
  vector<float> edges;
  float min = ranges_[0];
  float max = ranges_[1];
  int num_bins = bins_;
  float bin_width = (max - min) / num_bins;
  for (int i = 0; i <= num_bins; ++i) {
    edges.push_back(min + i * bin_width);
  }
  return edges;
}

void InspectorHistogram::OnNewFrame(Mat& frame) {
  auto& hist = CalculateHistogram(frame);
  RenderHistogram(hist);
}

const Mat& InspectorHistogram::CalculateHistogram(Mat& frame) {
  int width = mat_shape_[2];
  int height = mat_shape_[1];
  uint8_t* data = frame.data;
  if (channel_ == kAmplitudeChannel) {
    data = &frame.data[width * height * frame.elemSize()];
  }
  Mat m(height, width, mat_type_, data);
  Mat roi = m(roi_).clone();  // to make it continuous

  int nimages = 1;
  int channels[] = {0};
  Mat mask;
  int dims = 1;
  int histSize[] = {bins_};
  const float* ranges[] = {ranges_};

  // calcHist(&roi, nimages, channels, mask, histogram_, 1, histSize, ranges,
  // true,
  //          false);
  histogram_ = roi;
  return histogram_;
}