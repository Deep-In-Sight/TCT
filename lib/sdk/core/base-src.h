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

#ifndef __BASE_SRC_H__
#define __BASE_SRC_H__

#include <sdk/core/element.h>

#include <opencv2/opencv.hpp>
#include <string>

class Pad;

using namespace std;
using namespace cv;

class BaseSource : public Element {
 public:
  BaseSource(const string &name = "");
  ~BaseSource();

  /**
   * @brief Do nothing.
   *
   * @param frame: data from sink pad.
   */
  void PushFrame(Mat &frame);

  /**
   * @brief A loop that continuously send frame to its source pad. Default
   * implementation does not care about timing.
   *
   */
  virtual void PushFrame();

  /**
   * @brief Get the Source Pad.
   *
   * @return Pad*
   */
  Pad *GetSourcePad();

  void Start();
  void Stop();

 protected:
  /**
   * @brief Child element implement this method to generate the frame.
   *
   * @return Mat
   */
  virtual Mat GenerateFrame() = 0;

 private:
  Pad *source_pad_;
  bool running_;
};

#endif  //__BASE_SRC_H__
