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
#ifndef __DEPTH_CALC_H__
#define __DEPTH_CALC_H__

#include <sdk/core/base-transform.h>

class DepthCalc : public BaseTransform {
 public:
  DepthCalc(const string &name = "");
  ~DepthCalc();

  void SetConfig(float fmod, float offset);

 private:
  void TransformFrame(Mat &frame) override;
  void SetFrameFormat(const MatShape &shape, int type) override;

  float fmod_;
  float offset_;
};

#endif  //__DEPTH_CALC_H__