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

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <sdk/core/element.h>
#include <sdk/core/pad.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

/**
 * @brief Queue element. This element is used to queue frames from a source pad.
 * The purpose is to break the pipeline into two parts running in different
 * threads.
 *
 */
class Queue : public Element {
 public:
  /**
   * @brief Construct a new Queue object
   *
   * @param name
   */
  Queue(const string &name = "");
  ~Queue();

  /**
   * @brief Receive a frame from sink pad and put to Queue.
   * The PushFrame chain of all the elements up to the previous queue breaks
   * here.
   *
   * @param frame
   */
  void PushFrame(cv::Mat &frame);

  /**
   * @brief Get the source pad.
   *
   * @return Pad*
   */
  Pad *GetSourcePad();
  /**
   * @brief Get the sink pad.
   *
   * @return Pad*
   */
  Pad *GetSinkPad();

 private:
  thread *thread_;
  mutex mutex_;
  condition_variable condvar_;
  bool stop_thread_;
  queue<cv::Mat> queue_;
  Pad *src_;
  Pad *sink_;

  /**
   * @brief wait until there is new frame in queue, the start PushFrame chain.
   *
   */
  void WaitFrame();
};

#endif  // __QUEUE_H__