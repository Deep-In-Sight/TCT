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

#include <sdk/inspector/inspector-scanner.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;
static logger* logger_ = stdout_color_mt("InspectorScanner").get();

InspectorScanner::InspectorScanner(ScanDirection dir) {
  start_x_ = -1;
  start_y_ = -1;
  end_xy_ = -1;
  dir_ = dir;
}

void InspectorScanner::GetRoi(int& x1, int& y1, int& xy2) {
  x1 = start_x_;
  y1 = start_y_;
  xy2 = end_xy_;
}

void InspectorScanner::OnNewFrame(Mat& frame) {
  auto vec = CollectRange(frame);
  RenderRange(vec);
}

void InspectorScanner::SetRoi(int x1, int y1, int xy2) {
  size_t vec_size;

  start_x_ = std::min(mat_shape_[2] - 1, std::max(0, x1));
  start_y_ = std::min(mat_shape_[1] - 1, std::max(0, y1));
  if (dir_ == kScanHorizontal) {
    end_xy_ = std::min(mat_shape_[2] - 1, std::max(0, xy2));
    if (end_xy_ < start_x_) {
      std::swap(start_x_, end_xy_);
    }
    vec_size = end_xy_ - start_x_;
  } else {
    end_xy_ = std::min(mat_shape_[1] - 1, std::max(0, xy2));
    if (end_xy_ < start_y_) {
      std::swap(start_y_, end_xy_);
    }
    vec_size = end_xy_ - start_y_;
  }

  if (vec_size > collected_.capacity()) {
    collected_.reserve(vec_size);
  }
}

const std::vector<float>& InspectorScanner::CollectRange(Mat& frame) {
  collected_.clear();
  if (dir_ == kScanHorizontal) {
    for (int x = start_x_; x <= end_xy_; x++) {
      collected_.push_back(frame.at<float>(channel_, start_y_, x));
    }
  } else {
    for (int y = start_y_; y <= end_xy_; y++) {
      collected_.push_back(frame.at<float>(channel_, y, start_x_));
    }
  }
  return collected_;
}

InspectorHScanner::InspectorHScanner() : InspectorScanner(kScanHorizontal) {}

InspectorVScanner::InspectorVScanner() : InspectorScanner(kScanVertical) {}