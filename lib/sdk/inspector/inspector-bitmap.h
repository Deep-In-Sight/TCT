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

#ifndef __INSPECTOR_BITMAP_H__
#define __INSPECTOR_BITMAP_H__

#include <sdk/core/pad.h>

class InspectorBitmap : public PadObserver {
 public:
  InspectorBitmap(const std::string& name);
  /**
   * @brief Render the whole frame
   *
   * @param frame
   */
  void OnNewFrame(Mat& frame) override;

  /**
   * @brief Get the depth and amplitude at a specific pixel. This function can
   * be used in mouse event handler to peak the value under the cursor.
   *
   * @param x
   * @param y
   * @return Vec2f depth and amplitude. Amplitude can be NaN if Pad only has 1
   * channel.
   * @throw std::runtime_error if x or y is out of frame
   */
  Vec2f GetPixel(int x, int y);

 protected:
  /**
   * @brief Render the frame. To be called from OnNewFrame(Mat&). This function
   * is implemented in child class to render the depth/amplitude to either GUI
   * window for visualizing. Unlike other inspector, this one only forward the
   * data to GUI widget. It is the widget's job to do the colormap and implement
   * the mouse hover/click event to get the point value.
   *
   * @param buffer
   */
  virtual void Render(Mat& frame) = 0;
  Mat frame_;
};

#endif  // __INSPECTOR_BITMAP_H__