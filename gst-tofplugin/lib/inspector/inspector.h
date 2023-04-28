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

#include <gst/gst.h>

#include <string>
#include <vector>
#include <queue>

class InspectorClient {
 protected:
  virtual void update(void* data, size_t size) = 0;
};

class Inspector {
 public:
  Inspector(const std::string name);
  ~Inspector();

  int attach(GstPad* pad);
  void detach();
  int add_subscriber(InspectorClient* client);
  size_t get_num_samples();

 private:
  std::string name;

  int probe_id;
  GstPad* pad;
  std::vector<InspectorClient*> subscribers;
  std::queue<GstBuffer*> buffers;

  static GstPadProbeReturn QueueBuffer(GstPad* pad, GstPadProbeInfo* info,
                                       gpointer user_data);
};

#endif  //__INSPECTOR_H__