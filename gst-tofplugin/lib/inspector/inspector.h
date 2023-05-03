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

#include <condition_variable>
#include <list>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class InspectorClient {
 public:
  virtual void Update(GstBuffer* buffer) = 0;
};

class Inspector {
 public:
  Inspector(const std::string name);
  ~Inspector();

  int Attach(GstPad* pad);
  void Detach();
  void AddClient(InspectorClient* client);
  void RemoveClient(InspectorClient* client);

  size_t GetNumSamples();
  size_t GetNumClients();

 private:
  std::string name;

  int probe_id;
  GstPad* pad;
  std::list<InspectorClient*> clients;
  std::queue<GstBuffer*> buffers;

  static GstPadProbeReturn QueueBuffer(GstPad* pad, GstPadProbeInfo* info,
                                       gpointer user_data);

  std::thread* t;
  std::mutex mutex;
  std::condition_variable condvar;
  bool stop_inspecting;

 protected:
  void UpdateClients();
};

#endif  //__INSPECTOR_H__