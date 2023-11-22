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
#ifndef __INSPECTOR_SCANNER_H__
#define __INSPECTOR_SCANNER_H__

#include <sdk/core/pad.h>

#include <vector>

class InspectorScanner : public PadObserver {
 public:
  InspectorScanner();
  InspectorScanner(int x1, int y1, int x2, int y2);

  /**
   * @brief set the range of the scanner. This range is inclusive-start and
   * inclusive-end. The range is clampped by frame width and height. Also start
   * point and end point are swaped if they not not in ascending order.
   *
   * @param x1 start point x
   * @param y1 start point y
   * @param x2 end point x
   * @param y2 end point y
   */
  void SetRoi(int x1, int y1, int x2, int y2);

  /**
   * @brief Get the range of the scanner
   *
   * @param x1 start point x
   * @param y1 start point y
   * @param x2 end point x
   * @param y2 end point y
   */
  void GetRoi(int& x1, int& y1, int& x2, int& y2);

  void OnNewFrame(Mat& frame) override;

 protected:
  /**
   * @brief not to be called from outside. This function is called by
   * OnNewFrame(Mat&). protected for unittesting.
   *
   * @param frame Mat to collect data from
   * @return std::vector<float>& collected data
   */
  const vector<float>& CollectRange(Mat& frame);
  /**
   * @brief not to be called from outside. This fuction is called by
   * OnNewFrame(Mat&). Implemented by child class, to render the collected data,
   * either to a gui window to visualizing, or a text file for storing.
   *
   * @param vec
   */
  virtual void RenderRange(const std::vector<float>& vec) = 0;

  int start_x_;
  int start_y_;
  int end_x_;
  int end_y_;

  vector<float> collected_;

 private:
};

void liang_barsky_clipper(int& x1, int& y1, int& x2, int& y2, int xmin,
                          int ymin, int xmax, int ymax);

#endif  //__INSPECTOR_SCANNER_H__