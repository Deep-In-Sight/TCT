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

#ifndef __INSPECTOR_TRACKER_H__
#define __INSPECTOR_TRACKER_H__

#include <sdk/core/pad.h>

#include <opencv2/opencv.hpp>

class InspectorTracker : public PadObserver {
 public:
  /**
   * @brief Set the Location of the tracker
   *
   * @param x tracker's x coordinate
   * @param y tracker's y coordinate
   */
  void SetLocation(int x, int y);
  /**
   * @brief Get the Location of the tracker
   *
   * @param x tracker's x coordinate
   * @param y tracker's y coordinate
   */
  void GetLocation(int& x, int& y);

  void OnNewFrame(Mat& frame) override;

 protected:
  /**
   * @brief Get the Point value at tracker location. Not to be called directly,
   * it's called in Update() function
   *
   * @param buffer GstBuffer to get point value
   * @return float point value
   */
  float GetPoint(Mat& frame);
  /**
   * @brief Render the point value. Not to be called directly, it's called in
   * Update() function. This function is implemented in child class to render
   * the point value to either GUI window for visualizing, or text file for
   * storing.
   *
   * @param value
   */
  virtual void RenderPoint(float value) = 0;

 private:
  int point_x;
  int point_y;
  float point_val;
};

#endif  //__INSPECTOR_TRACKER_H__