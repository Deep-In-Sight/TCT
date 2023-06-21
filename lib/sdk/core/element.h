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

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <sdk/core/pad.h>

#include <iterator>
#include <list>
#include <opencv2/opencv.hpp>
#include <string>

// class Pad;
// class MatShape;

using namespace std;
using namespace cv;

typedef list<Pad *>::iterator PadIterator;

/**
 * @brief Base class for all elements. An element is a processing unit in a
 * pipeline. It has a list of pads which are used to connect to other elements.
 * Child class implement the PushFrame method to process the frame received from
 * sink pads, and possibly push the result to source pads, which in turn forward
 * to its peer pad.
 *
 */
class Element {
 public:
  /**
   * @brief Construct a new Element object.
   *
   * @param name Name of the element.
   */
  Element(const string &name = "");
  ~Element();

  /**
   * @brief Get the name of the element
   *
   * @return const string&
   */
  const string &GetName();

  /**
   * @brief Set the Name of the element.
   *
   * @param name
   */
  void SetName(const string &name);

  /**
   * @brief Get the pad with name. Return nullptr if not found.
   *
   * @param name Name of the pad.
   * @return Pad*
   */
  Pad *GetPad(const string &name);

  /**
   * @brief Push a frame to the element. The child class implement this method
   * to process the frame from sink pads. It may display the frame, or do some
   * transform and push the result to source pads.
   *
   * @param frame: data from sink pad.
   */
  virtual void PushFrame(Mat &frame) = 0;

  /**
   * @brief Handle the state change of the stream.
   * Child implement this to update the element state. Don't forget to propagate
   * downstream.
   */
  virtual void PushState(StreamState state){};

  /**
   * @brief Child element reimplement this method to transform the size and
   * type between sink to source pad. Default implementation send the same
   * size and type to all the sink pad.
   *
   * @param size
   * @param type
   */
  virtual void SetFrameFormat(const MatShape &shape, int type);

 protected:
  /**
   * @brief Add a pad to the element. The pad must have a unique name.
   * This function will also add the element as the parent of the pad.
   *
   * @param pad
   * @return true
   * @return false
   */
  bool AddPad(Pad *pad);

 private:
  string name_;
  list<Pad *> pads_;
};

#endif  // __ELEMENT_H__