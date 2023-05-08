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

#include <lib/inspector/inspector-client.h>

#include <vector>

class InspectorScanner : public InspectorClient {
 public:
  enum ScanDirection { kScanHorizontal = 0, kScanVertical };
  /**
   * @brief set the range of the scanner. This range is inclusive-start and
   * inclusive-end. The range is clampped by frame width and height. Also start
   * point and end point are swaped if they not not in ascending order.
   *
   * @param x1 start point x
   * @param y1 start point y
   * @param xy2 end point x or y
   */
  virtual void SetRange(int x1, int y1, int xy2) = 0;
  /**
   * @brief Get the range of the scanner
   *
   * @param x1 start point x
   * @param y1 start point y
   * @param xy2 end point x or y
   */
  void GetRange(int& x1, int& y1, int& xy2);

  void Update(GstBuffer* buffer) override;

 protected:
  /**
   * @brief not to be called from outside. This function is called by Update().
   * Implemented in InspectorHScanner and InspectorVScanner to collect data from
   * GstBuffer.
   *
   * @param buffer GstBuffer to collect data from
   * @return std::vector<float>& collected data
   */
  virtual std::vector<float>& CollectRange(GstBuffer* buffer) = 0;
  /**
   * @brief not to be called from outside. This function is called by Update().
   * Implemented by child class, to render the collected data, either to a gui
   * window to visualizing, or a text file for storing.
   *
   * @param vec
   */
  virtual void RenderResult(std::vector<float>& vec) = 0;
  /**
   * @brief not to be called from outside. This function is called by
   * SetRange() in each child class.
   *
   * @param x1 start point x
   * @param y1 start point y
   * @param xy2 end point x or y
   * @param dir scan direction
   */
  void SetRangeImpl(int x1, int y1, int xy2, ScanDirection dir);
  /**
   * @brief not to be called from outside. This function is called by
   * CollectRange() in each child class.
   *
   * @param buffer GstBuffer to collect data from
   * @param dir scan direction
   * @return std::vector<float>& collected data
   */
  std::vector<float>& CollectRangeImpl(GstBuffer* buffer, ScanDirection dir);

 private:
  int start_x;
  int start_y;
  int end_xy;
  std::vector<float> collected;
};

class InspectorHScanner : public InspectorScanner {
 public:
  void SetRange(int x1, int y1, int xy2) override;

 private:
  std::vector<float>& CollectRange(GstBuffer* buffer) override;
};

class InspectorVScanner : public InspectorScanner {
 public:
  void SetRange(int x1, int y1, int xy2) override;

 private:
  std::vector<float>& CollectRange(GstBuffer* buffer) override;
};

#endif  //__INSPECTOR_SCANNER_H__