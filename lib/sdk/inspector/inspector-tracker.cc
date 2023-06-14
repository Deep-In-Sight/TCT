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
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;
static logger* logger_ = stdout_color_mt("InspectorTracker").get();

InspectorTracker::InspectorTracker() {
  point_x = mat_shape_[2] / 2;
  point_y = mat_shape_[1] / 2;
}

void InspectorTracker::SetLocation(int x, int y) {
  point_x = std::max(0, std::min(x, mat_shape_[2] - 1));
  point_y = std::max(0, std::min(y, mat_shape_[1] - 1));
}

void InspectorTracker::GetLocation(int& x, int& y) {
  x = point_x;
  y = point_y;
}

void InspectorTracker::OnNewFrame(Mat& frame) {
  point_val = GetPoint(frame);
  RenderPoint(point_val);
}

float InspectorTracker::GetPoint(Mat& frame) {
  float value;
  value = frame.at<float>(channel_, point_y, point_x);
  return value;
}
