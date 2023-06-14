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
#ifndef __INSPECTOR_H__
#define __INSPECTOR_H__

#include <sdk/core/pad.h>
#include <sdk/core/queue.h>

#include <list>
#include <string>

/**
 * @brief InspectorQueue is a PadObserver with a queue that store the data and
 * then notify other PadObserver(s). It is used in case the observers takes long
 * time to process data.
 *
 */
class InspectorQueue : public PadObserver {
 public:
  InspectorQueue(const std::string name);
  ~InspectorQueue();

  void AddInspector(PadObserver* inspector);
  void RemoveInspector(PadObserver* inspector);

  void OnNewFrame(cv::Mat& frame) override;
  void OnFrameFormatChanged(const MatShape& shape, int type) override;

 private:
  std::string name_;
  Queue queue_;
};

#endif  //__INSPECTOR_H__