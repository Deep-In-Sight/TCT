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
#include <lib/inspector/inspector.h>

#include <iostream>

Inspector::Inspector(const std::string name) { this->name = name; }

Inspector::~Inspector() { ; }

int Inspector::attach(GstPad* pad) {
  int probe_id;
  probe_id = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                               Inspector::QueueBuffer, this, NULL);
  this->probe_id = probe_id;
  this->pad = pad;
  return probe_id;
}

void Inspector::detach() { gst_pad_remove_probe(this->pad, this->probe_id); }

int Inspector::add_subscriber(InspectorClient* client) {
  int num_samples;
  subscribers.push_back(client);
  num_samples = subscribers.size();
  return num_samples;
}

size_t Inspector::get_num_samples() {
  size_t num_samples;
  num_samples = buffers.size();
  return num_samples;
}

GstPadProbeReturn Inspector::QueueBuffer(GstPad* pad, GstPadProbeInfo* info,
                                         gpointer user_data) {
  Inspector* inspector = (Inspector*)user_data;
  GstBuffer* buffer = gst_pad_probe_info_get_buffer(info);
  buffer = gst_buffer_ref(buffer);
  inspector->buffers.push(buffer);
  /*wake up update loop*/

  return GST_PAD_PROBE_OK;
}