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

#include <sdk/inspector/inspector-bitmap.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;
static logger* logger_ = stdout_color_mt("InspectorBitmap").get();

void InspectorBitmap::OnNewFrame(Mat& frame) {
  frame_ = frame;
  Render(frame);
}

Vec2f InspectorBitmap::GetPixel(int x, int y) {
  if (x < 0 || x >= frame_.size[2] || y < 0 || y >= frame_.size[1]) {
    throw std::runtime_error("out of frame");
  }

  if (mat_shape_.dims() == 1) {
    return Vec2f(frame_.at<float>(0, y, x), nanf(""));
  } else {
    return Vec2f(frame_.at<float>(0, y, x), frame_.at<float>(1, y, x));
  }
}