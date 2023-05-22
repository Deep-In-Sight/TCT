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

#ifndef __BASE_TRANSFORM_H__
#define __BASE_TRANSFORM_H__

#include <sdk/core/element.h>

#include <opencv2/opencv.hpp>
#include <string>

class Pad;

using namespace std;
using namespace cv;

/**
 * @brief A transform element that has one sink and one source pad.
 *
 */
class BaseTransform : public Element {
 public:
  BaseTransform(const string &name = "");
  ~BaseTransform();

  /**
   * @brief Transform the frame.
   *
   * @param frame: data from sink pad.
   */
  void PushFrame(Mat &frame) override;

  Pad *GetSinkPad();
  Pad *GetSourcePad();

 protected:
  /**
   * @brief Child transform class implement this method to transform the frame.
   * Default implementation forward the frame to source pad.
   *
   * @param frame
   */
  virtual void TransformFrame(Mat &frame);

 private:
  Pad *sink_pad_;
  Pad *source_pad_;
};
#endif  //__BASE_TRANSFORM_H__
