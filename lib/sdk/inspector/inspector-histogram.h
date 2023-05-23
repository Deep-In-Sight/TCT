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

#include <sdk/core/pad.h>

#include <vector>

/**
 * @brief InspectorHistogram is a subclass of InspectorClient. It calculates the
 * histogram of a GstBuffer and renders it. The user has to guess the range of
 * the histogram, and call SetBins() to set the bins' edges.
 *
 */
class InspectorHistogram : public PadObserver {
 public:
  /**
   * @brief Construct a new InspectorHistogram object.
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
  void SetRoi(int x, int y, int x2, int y2);
  /**
   * @brief Get the clamped/swapped range of the histogram
   *
   * @param x start point x
   * @param y start point y
   * @param x2 end point x
   * @param y2 end point y
   */
  void GetRoi(int& x, int& y, int& x2, int& y2);
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
   * @return vector<float> bins' edges
   */
  vector<float> GetEdges();

  /**
   * @brief Calculate the histogram and render it.
   *
   * @param buffer
   */
  void OnNewFrame(Mat& frame) override;

 protected:
  /**
   * @brief calculate histogram. This function is called by from
   * OnNewFrame(Mat&) and should not be called from children. It is made
   * protected only for unit test.
   *
   * @param frame input matrix to calculate histogram
   * @return HistogramData& histogram data
   */
  const Mat& CalculateHistogram(Mat& frame);
  /**
   * @brief Render the histogram. This function is called by OnNewFrame(Mat&)
   * and should not be called directly. It is only made protected so it can be
   * mocked away for unit test. Child class implements this function to render
   * the histogram on a GUI window or to a text file.
   *
   * @param histogram
   */
  virtual void RenderHistogram(const Mat& histogram) = 0;

 private:
  Rect roi_;
  float ranges_[2];
  int bins_;
  Mat histogram_;
};

#endif  //__INSPECTOR_HISTOGRAM_H__