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
#include <sdk/core/queue.h>

#include <condition_variable>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

class Pad;

using namespace std;
using namespace cv;

enum StreamState {
  kStreamStatePlaying,
  kStreamStatePaused,
  kStreamStateStopped
};

class BaseSource : public Element {
 public:
  /**
   * @brief Construct a new BaseSource object
   *
   * @param name name of the source
   * @param is_async whether or not to add a queue to decouple the source
   * reading thread from the rest of the pipeline.
   */
  BaseSource(const string &name = "", bool is_async = false);
  ~BaseSource();

  /**
   * @brief Do nothing.
   *
   * @param frame: data from sink pad.
   */
  void PushFrame(Mat &frame) override;

  /**
   * @brief A loop that continuously send frame to its source pad. Default
   * implementation does not care about timing.
   *
   */
  void GenerateLoop();

  /**
   * @brief Get the Source Pad.
   *
   * @return Pad*
   */
  Pad *GetSourcePad();

  bool Start();
  bool Stop();
  bool Pause();
  bool Resume();
  bool Step();
  StreamState GetState();

 protected:
  /**
   * @brief Child element implement this method to generate the frame.
   *
   * @return Mat
   */
  virtual Mat GenerateFrame() = 0;
  /**
   * @brief Child element implement this to initialize source, such as configure
   * the camera sensor (live streaming), or open a file descriptor (playback).
   *
   */
  virtual bool InitializeSource() = 0;
  /**
   * @brief Child element implement this to clean up the source. For example
   * put camera on standby or close the file descriptor.
   *
   * @return
   */
  virtual void CleanupSource() = 0;

 private:
  Pad *source_pad_;
  Queue *queue_;
  thread *thread_;
  StreamState state_;
  mutex mutex_;
  condition_variable condvar_;
  float duration_;
};

#endif  //__BASE_SRC_H__
