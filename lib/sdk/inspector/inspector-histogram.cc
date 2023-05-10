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
  top_left_x = 0;
  top_left_y = 0;
  bottom_right_x = 0;
  bottom_right_y = 0;
  width = 0;
  height = 0;
  is_amplitude = false;
  is_uniform = true;
  histogram_data.counts = std::vector<int>(1, 0);
  histogram_data.edges = std::vector<float>({
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::max(),
  });
}

void InspectorHistogram::SetRange(int x, int y, int x2, int y2) {
  top_left_x = std::max(0, std::min(x, width - 1));
  top_left_y = std::max(0, std::min(y, height - 1));
  bottom_right_x = std::max(0, std::min(x2, width - 1));
  bottom_right_y = std::max(0, std::min(y2, height - 1));
  if (top_left_x > bottom_right_x) {
    std::swap(top_left_x, bottom_right_x);
  }
  if (top_left_y > bottom_right_y) {
    std::swap(top_left_y, bottom_right_y);
  }
}

void InspectorHistogram::GetRange(int& x, int& y, int& x2, int& y2) {
  x = top_left_x;
  y = top_left_y;
  x2 = bottom_right_x;
  y2 = bottom_right_y;
}

void InspectorHistogram::SetBins(const std::vector<float>& edges) {
  is_uniform = false;

  std::vector<float> edges_ext = edges;
  std::sort(edges_ext.begin(), edges_ext.end());

  edges_ext.insert(edges_ext.begin(), std::numeric_limits<float>::lowest());
  edges_ext.push_back(std::numeric_limits<float>::max());
  SetEdges_(edges_ext);
}

void InspectorHistogram::SetBins(float min, float max, int num_bins) {
  float bin_width = (max - min) / num_bins;
  std::vector<float> edges;
  edges.push_back(std::numeric_limits<float>::lowest());
  for (int i = 0; i <= num_bins; ++i) {
    edges.push_back(min + i * bin_width);
  }
  edges.push_back(std::numeric_limits<float>::max());
  this->is_uniform = true;
  SetEdges_(edges);
}

void InspectorHistogram::SetEdges_(std::vector<float>& edges) {
  this->histogram_data.edges = edges;
  int num_bins = edges.size() - 1;
  this->histogram_data.counts = std::vector<int>(num_bins, 0);
}

std::vector<float>& InspectorHistogram::GetEdges() {
  return histogram_data.edges;
}

void InspectorHistogram::Update(GstBuffer* buffer) {
  const HistogramData& hist = GetHistogram(buffer);
  RenderHistogram(hist);
}

const HistogramData& InspectorHistogram::GetHistogram(GstBuffer* buffer) {
  GstMapInfo mapinfo;
  gst_buffer_map(buffer, &mapinfo, GST_MAP_READ);
  float* data = reinterpret_cast<float*>(mapinfo.data);

  int num_edges = histogram_data.edges.size();
  int num_bins = num_edges - 1;
  float min_val = histogram_data.edges[1];
  float max_val = histogram_data.edges[num_edges - 2];
  float bin_width = (max_val - min_val) / (num_bins - 2);

  // reset counts
  std::vector<int>& counts = histogram_data.counts;
  for (int bin = 0; bin < num_bins; ++bin) {
    counts[bin] = 0;
  }

  // 0             1    2    3             4
  // |     0       | 1  | 2  |      3      |
  if (!this->is_uniform) {
    for (int y = top_left_y; y <= bottom_right_y; ++y) {
      for (int x = top_left_x; x <= bottom_right_x; ++x) {
        int pixel_count = y * width + x;
        float value = data[pixel_count];
        int bin = 0;
        while ((bin + 1 < num_edges) &&
               (value >= histogram_data.edges[bin + 1])) {
          ++bin;
        }
        ++counts[bin];
      }
    }
  } else {
    for (int y = top_left_y; y <= bottom_right_y; ++y) {
      for (int x = top_left_x; x <= bottom_right_x; ++x) {
        int pixel_count = y * width + x;
        float value = data[pixel_count];
        int bin = 0;
        bin = std::ceil((value - min_val) / bin_width);
        if (bin * bin_width + min_val == value) {
          bin += 1;
        }
        bin = std::max(0, std::min(bin, num_bins - 1));
        ++counts[bin];
      }
    }
  }

  gst_buffer_unmap(buffer, &mapinfo);
  return histogram_data;
}