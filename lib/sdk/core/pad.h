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

#ifndef __PAD_H__
#define __PAD_H__

#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;

class Element;

/**
 * @brief PadDirection A Pad is either a source or a sink pad. A source pad
 * receives data from the element then send to its peer. A sink pad receives
 * data from its peer then send to the element. A source pad can only be linked
 * to a sink pad and vice versa.
 *
 */
enum PadDirection { kPadSource, kPadSink };

/**
 * @brief PadLinkStatus A Pad can be linked or unlinked. A linked pad has a
 * peer. Same direction can not linked together.
 *
 */
enum PadLinkStatus { kPadUnlinked, kPadLinked };

/**
 * @brief PadLinkReturn Return value of Pad::Link method.
 *
 */
enum PadLinkReturn {
  kPadLinkOk,
  kPadLinkWrongDirection,
  kPadLinkAlreadyLinked
};

class PadObserver {
 public:
  virtual void OnNewFrame(cv::Mat &frame) = 0;
  void SetSizeType(Size size, int type);

 private:
  Size mat_size_;
  int mat_type_;
};

/**
 * @brief A Pad is a connection point between elements. It has a direction and a
 * name. A Pad can be linked to another Pad of opposite direction. A Pad can be
 * a source or a sink.
 *
 */
class Pad {
 public:
  /**
   * @brief Construct a new Pad object
   *
   * @param direction
   * @param name
   */
  Pad(PadDirection direction, const string &name);
  ~Pad();

  /**
   * @brief Set the name of the pad
   *
   * @param name
   */
  void SetName(const string &name);
  /**
   * @brief Get the name of the pad
   *
   * @return const string&
   */
  const string &GetName();
  /**
   * @brief Get the Direction of the pad.
   *
   * @return PadDirection
   */
  PadDirection GetDirection();
  /**
   * @brief Get the Link status of the pad.
   *
   * @return PadLinkStatus
   */
  PadLinkStatus GetLinkStatus();
  /**
   * @brief Get the parent element of the pad.
   *
   * @return Element*
   */
  Element *GetParent();
  /**
   * @brief Set the parent element of the pad. This method should only be called
   * in Element::AddPad.
   *
   * @param parent
   * @return true
   * @return false
   */
  bool SetParent(Element *parent);
  /**
   * @brief link a pad with another pad. The other pad must be of opposite
   *
   * @param peer
   * @return PadLinkReturn
   */
  PadLinkReturn Link(Pad *peer);
  /**
   * @brief Unlink the pad with its peer.
   *
   * @return PadLinkReturn
   */
  PadLinkReturn Unlink();
  /**
   * @brief Get the peer pad.
   *
   * @return Pad*
   */
  Pad *GetPeer();
  /**
   * @brief Push a frame to its parent or peer pad.
   *
   * @param frame
   */
  void PushFrame(cv::Mat &frame);

  /**
   * @brief Add an observer to the pad. The observer will be notified when a new
   * frame is pushed to the pad.
   *
   * @param observer
   */
  void AddObserver(PadObserver *observer);

  /**
   * @brief Remove an observer from the pad.
   *
   * @param observer
   */
  void RemoveObserver(PadObserver *observer);

  /**
   * @brief Set the size and type of the data that the pad will receive. Also
   * update all downstream pads and elements type and size.
   *
   * @param mat_size
   * @param mat_type
   */
  void SetSizeType(Size mat_size, int mat_type);

  /**
   * @brief Get the size and type of the data that the pad will receive.
   *
   * @param mat_size
   * @param mat_type
   */
  void GetSizeType(Size &mat_size, int &mat_type);

 private:
  PadDirection direction_;
  PadLinkStatus link_status_;
  Element *parent_;
  Pad *peer_;
  list<PadObserver *> observers_;
  string name_;

  Size mat_size_;
  int mat_type_;
};

#endif /* __PAD_H__ */