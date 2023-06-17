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

enum ScanDirection { kScanHorizontal = 0, kScanVertical };

class InspectorScanner : public PadObserver {
 public:
  InspectorScanner(ScanDirection dir = kScanHorizontal);

  /**
   * @brief set the range of the scanner. This range is inclusive-start and
   * inclusive-end. The range is clampped by frame width and height. Also start
   * point and end point are swaped if they not not in ascending order.
   *
   * @param x1 start point x
   * @param y1 start point y
   * @param xy2 end point x or y
   */
  void SetRoi(int x1, int y1, int xy2);

  /**
   * @brief Get the range of the scanner
   *
   * @param x1 start point x
   * @param y1 start point y
   * @param xy2 end point x or y
   */
  void GetRoi(int& x1, int& y1, int& xy2);

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

 private:
  int start_x_;
  int start_y_;
  int end_xy_;
  ScanDirection dir_;

  vector<float> collected_;
};

class InspectorHScanner : public InspectorScanner {
 public:
  InspectorHScanner();
};

class InspectorVScanner : public InspectorScanner {
 public:
  InspectorVScanner();
};

#endif  //__INSPECTOR_SCANNER_H__