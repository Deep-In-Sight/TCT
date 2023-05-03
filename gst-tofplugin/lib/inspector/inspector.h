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
#include <lib/inspector/inspector-client.h>

#include <condition_variable>
#include <list>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class InspectorHScanner : public InspectorClient {
 public:
  void SetRange(int x, int y, int x2);

 protected:
  virtual void RenderResult(std::vector<float>& row);
};

class InspectorVScanner : public InspectorClient {
 public:
  void SetRange(int x, int y, int y2);

 protected:
  virtual void RenderResult(std::vector<float>& col);
};

class InspectorTracker : public InspectorClient {
 public:
  void SetRange(int x, int y);

 protected:
  virtual void RenderResult(float point);
};

class InspectorHistogram : public InspectorClient {
 public:
  void SetRange(int x, int y, int x2, int y2);

 protected:
  virtual void RenderResult(std::vector<float>& count,
                            std::vector<float>& edges);
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

  std::vector<int> probe_ids;
  GstPad* pad;
  std::list<InspectorClient*> clients;
  std::queue<GstBuffer*> buffers;

  static GstPadProbeReturn NewBufferCallback(GstPad* pad, GstPadProbeInfo* info,
                                             gpointer user_data);
  static GstPadProbeReturn NewEventCallback(GstPad* pad, GstPadProbeInfo* info,
                                            gpointer user_data);

  std::thread* t;
  std::mutex mutex;
  std::condition_variable condvar;
  bool stop_inspecting;

 protected:
  void UpdateClients();
};

#endif  //__INSPECTOR_H__