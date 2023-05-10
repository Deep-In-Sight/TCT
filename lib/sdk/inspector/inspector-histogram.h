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

#ifndef __INSPECTOR_HISTOGRAM_H__
#define __INSPECTOR_HISTOGRAM_H__

#include <lib/inspector/inspector-client.h>

#include <vector>

struct HistogramData {
  std::vector<float> edges;
  std::vector<int> counts;
};

/**
 * @brief InspectorHistogram is a subclass of InspectorClient. It calculates the
 * histogram of a GstBuffer and renders it. The user has to guess the range of
 * the histogram, and call SetBins() to set the bins' edges.
 *
 */
class InspectorHistogram : public InspectorClient {
 public:
  /**
   * @brief Construct a new InspectorHistogram object. The default range is
   * (0,0),(0,0) with 1 bin from -inf to inf
   *
   */
  InspectorHistogram();
  virtual ~InspectorHistogram(){};
  /**
   * @brief Set the range to calculate histogram. This range is inclusive-start
   * and inclusive-end. The range is clamped to the frame size. If x2 < x or y2
   * < y, the start and end values are swapped.
   *
   * @param x start point x
   * @param y start point y
   * @param x2 end point x
   * @param y2 end point y
   */
  void SetRange(int x, int y, int x2, int y2);
  /**
   * @brief Get the clamped/swapped range of the histogram
   *
   * @param x start point x
   * @param y start point y
   * @param x2 end point x
   * @param y2 end point y
   */
  void GetRange(int& x, int& y, int& x2, int& y2);
  /**
   * @brief Set the bins' edges of the histogram. The edges are inclusive-start
   * and exclusive-end. The bins are (-inf, edges[0]), [edges[0], edges[1]),
   * ..., [edges[n-1], inf).
   *
   * @param edges bins' edges
   */
  void SetBins(const std::vector<float>& edges);
  /**
   * @brief Set the bins' edges of the histogram. The edges are inclusive-start
   * and exclusive-end. The [min, max] range is uniformly divided into num_bins,
   * with bin_width = (max - min) / num_bins. The bins' edges are (-inf, min),
   * [min, min+bin_width), ..., [max-bin_width, max), [max, inf).
   *
   * @param min minimum value of the histogram's edges
   * @param max maximum value of the histogram's edges
   * @param num_bins number of bins
   */
  void SetBins(float min, float max, int num_bins);

  /**
   * @brief Get the bins' edges of the histogram
   *
   * @return std::vector<float>& bins' edges
   */
  std::vector<float>& GetEdges();

  /**
   * @brief Implement InspectorClient::Update() to calculate histogram from
   * GstBuffer and render it.
   *
   * @param buffer
   */
  void Update(GstBuffer* buffer) override;

 protected:
  /**
   * @brief calculate histogram from GstBuffer. This function is called by
   * Update() and should not be called directly. It is made protected only for
   * unit test.
   *
   * @param buffer GstBuffer to calculate histogram
   * @return HistogramData& histogram data
   */
  const HistogramData& GetHistogram(GstBuffer* buffer);
  /**
   * @brief Render the histogram. This function is called by Update() and should
   * not be called directly. It is only made protected so it can be mocked away
   * for unit test. Child class implements this function to render the histogram
   * on a GUI window or to a text file.
   *
   * @param histogram
   */
  virtual void RenderHistogram(const HistogramData& histogram) = 0;

 private:
  void SetEdges_(std::vector<float>& edges);
  int top_left_x;
  int top_left_y;
  int bottom_right_x;
  int bottom_right_y;
  bool is_uniform;
  HistogramData histogram_data;
};

#endif  //__INSPECTOR_HISTOGRAM_H__