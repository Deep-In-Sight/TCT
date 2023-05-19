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

#ifndef __BASE_SINK_H__
#define __BASE_SINK_H__

#include <sdk/core/element.h>

#include <opencv2/opencv.hpp>
#include <string>

class Pad;

using namespace std;
using namespace cv;

/**
 * @brief A sink element with only sink pad that end the pipeline.
 *
 */

class BaseSink : public Element {
 public:
  BaseSink(const string &name = "");
  ~BaseSink();

  /**
   * @brief
   *
   * @param frame: data from sink pad.
   */
  void PushFrame(Mat &frame);

 protected:
  /**
   * @brief consume the frame. Child element implement this method to process
   * the frame.
   *
   * @param frame
   */
  virtual void SinkFrame(Mat &frame) = 0;

  Pad *GetSinkPad();

 protected:
  Pad *sink_pad_;
};

#endif  //__BASE_SINK_H__
