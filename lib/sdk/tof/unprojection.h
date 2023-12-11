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

#pragma once

#include <sdk/core/base-transform.h>

class PinholeParams {
 public:
  PinholeParams();
  PinholeParams(float fx, float fy, float cx, float cy, float dx, float dy);
  static PinholeParams DefaultParams();
  // focal length x (unit: mm)
  float fx_;
  // focal length y (unit: mm)
  float fy_;
  // optical center x (unit: pixel)
  float cx_;
  // optical center y (unit: pixel)
  float cy_;
  // pixel size x (unit: mm)
  float dx_;
  // pixel size y (unit: mm)
  float dy_;
  static const float default_fx;
  static const float default_fy;
  static const float default_cx;
  static const float default_cy;
  static const float default_dx;
  static const float default_dy;
};

class Unprojection : public BaseTransform {
 public:
  Unprojection(const string &name = "");
  ~Unprojection();

  void SetParams(PinholeParams &params);
  PinholeParams GetParams();

 private:
  void TransformFrame(Mat &frame) override;
  void SetFrameFormat(const MatShape &shape, int type) override;
  PinholeParams params_;
  // Mat cloudCache_;
  // Mat coeffCache_;
};
