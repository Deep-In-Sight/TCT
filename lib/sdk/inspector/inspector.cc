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
/**
 * SECTION: inspector
 *
 * Add description later
 *
 */
#include <sdk/inspector/inspector.h>

#include <iostream>

Inspector::Inspector(const std::string name) {
  this->name = name;
  stop_inspecting = false;
  t = new std::thread(&Inspector::UpdateClients, this);
  this->pad = NULL;
}

Inspector::~Inspector() {
  stop_inspecting = true;
  condvar.notify_one();
  t->join();

  /* flush all remaining buffers */
  while (!buffers.empty()) {
    GstBuffer* buf = buffers.front();
    buffers.pop();
    gst_buffer_unref(buf);
  }

  Detach();
}

int Inspector::Attach(GstPad* pad) {
  int probe_id;
  probe_id = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                               &Inspector::NewBufferCallback, this, NULL);
  this->probe_ids.push_back(probe_id);
  probe_id = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_EVENT_BOTH,
                               &Inspector::NewEventCallback, this, NULL);
  this->probe_ids.push_back(probe_id);

  this->pad = pad;
  return probe_id;
}

void Inspector::Detach() {
  if (this->pad) {
    for (auto id : probe_ids) {
      gst_pad_remove_probe(this->pad, id);
    }
    this->pad = NULL;
  }
}

void Inspector::AddClient(InspectorClient* client) {
  clients.push_back(client);
}

void Inspector::RemoveClient(InspectorClient* client) {
  clients.remove(client);
}

size_t Inspector::GetNumSamples() {
  size_t num_samples;
  std::lock_guard<std::mutex> lock(mutex);
  num_samples = buffers.size();
  return num_samples;
}

size_t Inspector::GetNumClients() {
  size_t num_subscribers;
  num_subscribers = clients.size();
  return num_subscribers;
}

GstPadProbeReturn Inspector::NewBufferCallback(GstPad* pad,
                                               GstPadProbeInfo* info,
                                               gpointer user_data) {
  Inspector* inspector = (Inspector*)user_data;
  GstBuffer* buffer = gst_pad_probe_info_get_buffer(info);
  buffer = gst_buffer_ref(buffer);
  {
    std::lock_guard<std::mutex> lock(inspector->mutex);
    inspector->buffers.push(buffer);
  }
  /*wake up update loop*/
  inspector->condvar.notify_one();
  return GST_PAD_PROBE_OK;
}

/* event handler consume the event and caps, unless it forwards to others */
GstPadProbeReturn Inspector::NewEventCallback(GstPad* pad,
                                              GstPadProbeInfo* info,
                                              gpointer user_data) {
  Inspector* inspector = (Inspector*)user_data;
  GstEvent* event = gst_pad_probe_info_get_event(info);
  GstCaps* caps = NULL;

  switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_CAPS:
      gst_event_parse_caps(event, &caps);

      for (auto client : inspector->clients) {
        client->SetCaps(caps);
      }
      break;
    default:
      break;
  }

  return GST_PAD_PROBE_OK;
}

void Inspector::UpdateClients() {
  GstBuffer* buffer;
  while (!stop_inspecting) {
    {
      std::unique_lock<std::mutex> lock(mutex);
      /* check to prevent spurious wake */
      condvar.wait(lock, [&] { return stop_inspecting || !buffers.empty(); });

      if (stop_inspecting) {
        break;
      }

      buffer = buffers.front();
      buffers.pop();
    }

    for (auto client : clients) {
      /* do something time comsuming with the buffer */
      client->Update(buffer);
    }

    gst_buffer_unref(buffer);
  }
}