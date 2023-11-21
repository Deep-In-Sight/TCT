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

InspectorScanner::InspectorScanner() {
  start_x_ = -1;
  start_y_ = -1;
  end_x_ = -1;
  end_y_ = -1;
}

InspectorScanner::InspectorScanner(int x1, int y1, int x2, int y2) {
  SetRoi(x1, y1, x2, y2);
}

void InspectorScanner::GetRoi(int& x1, int& y1, int& x2, int& y2) {
  x1 = start_x_;
  y1 = start_y_;
  x2 = end_x_;
  y2 = end_y_;
}

void InspectorScanner::OnNewFrame(Mat& frame) {
  auto vec = CollectRange(frame);
  RenderRange(vec);
}

void InspectorScanner::SetRoi(int x1, int y1, int x2, int y2) {
  int width = mat_shape_[2];
  int height = mat_shape_[1];

  start_x_ = x1;
  start_y_ = y1;
  end_x_ = x2;
  end_y_ = y2;

  liang_barsky_clipper(start_x_, start_y_, end_x_, end_y_, 0, 0, width - 1,
                       height - 1);
}

const std::vector<float>& InspectorScanner::CollectRange(Mat& frame) {
  collected_.clear();

  int dx = end_x_ - start_x_;
  int dy = end_y_ - start_y_;

  int steps = max(abs(dx), abs(dy));

  float Xinc = dx / (float)steps;
  float Yinc = dy / (float)steps;

  float X = start_x_;
  float Y = start_y_;
  for (int i = 0; i <= steps; i++) {
    // round up to nearest pixel
    auto x = (int)(X + 0.5);
    auto y = (int)(Y + 0.5);
    auto val = frame.at<float>(channel_, y, x);
    collected_.push_back(val);

    X += Xinc;
    Y += Yinc;
  }

  return collected_;
}

void liang_barsky_clipper(int& x1, int& y1, int& x2, int& y2, int xmin,
                          int ymin, int xmax, int ymax) {
  int dx = x2 - x1;
  int dy = y2 - y1;

  double p[] = {-dx, dx, -dy, dy};
  double q[] = {x1 - xmin, xmax - x1, y1 - ymin, ymax - y1};

  double u1 = 0.0, u2 = 1.0;

  for (int i = 0; i < 4; i++) {
    if (p[i] == 0) {
      if (q[i] < 0) {
        x1 = x2 = y1 = y2 = 0;  // line is outside of the rectangle
        return;
      }
    } else {
      double t = q[i] / p[i];
      if (p[i] < 0 && u1 < t)
        u1 = t;
      else if (p[i] > 0 && u2 > t)
        u2 = t;
    }
  }

  if (u1 > u2) {
    x1 = x2 = y1 = y2 = 0;  // line is outside of the rectangle
  } else {
    x2 = x1 + u2 * dx;
    y2 = y1 + u2 * dy;
    x1 = x1 + u1 * dx;
    y1 = y1 + u1 * dy;
  }
}