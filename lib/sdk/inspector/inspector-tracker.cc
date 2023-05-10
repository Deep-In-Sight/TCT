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

#include <sdk/inspector/inspector-tracker.h>

void InspectorTracker::SetLocation(int x, int y) {
  point_x = std::max(0, std::min(x, width - 1));
  point_y = std::max(0, std::min(y, height - 1));
}

void InspectorTracker::GetLocation(int& x, int& y) {
  x = point_x;
  y = point_y;
}

void InspectorTracker::Update(GstBuffer* buffer) {
  point_val = GetPoint(buffer);
  RenderPoint(point_val);
}

float InspectorTracker::GetPoint(GstBuffer* buffer) {
  GstMapInfo mapinfo;
  gst_buffer_map(buffer, &mapinfo, GST_MAP_READ);
  float* data = reinterpret_cast<float*>(mapinfo.data);

  int index = point_y * width + point_x;
  float value = data[index];

  gst_buffer_unmap(buffer, &mapinfo);
  return value;
}
