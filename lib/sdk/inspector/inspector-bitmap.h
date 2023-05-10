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

#include <lib/inspector/inspector-client.h>

class InspectorBitmap : public InspectorClient {
 public:
  /**
   * @brief Update the bitmap
   *
   * @param buffer
   */
  void Update(GstBuffer* buffer) override;

 protected:
  /**
   * @brief Render the GstBuffer's data. Not to be called directly, it's called
   * in Update() function. This function is implemented in child class to render
   * the depth/amplitude to either GUI window for visualizing. Unlike other
   * inspector, this one only forward the data to GUI widget. It is the widget's
   * job to do the colormap and implement the mouse hover/click event to get the
   * point value.
   *
   * @param buffer
   */
  virtual void Render(float* data) = 0;
};

#endif  // __INSPECTOR_BITMAP_H__