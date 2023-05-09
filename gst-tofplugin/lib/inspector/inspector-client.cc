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

#include <lib/inspector/inspector-client.h>

#include <iostream>

void InspectorClient::SetCaps(GstCaps *caps) {
  int buffer_width;
  int buffer_height;

  GstStructure *caps_struct = gst_caps_get_structure(caps, 0);
  const gchar *buffer_format = gst_structure_get_string(caps_struct, "format");
  gst_structure_get_int(caps_struct, "width", &buffer_width);
  gst_structure_get_int(caps_struct, "height", &buffer_height);
  format = std::string(buffer_format);
  this->width = buffer_width;
  this->height = buffer_height;
}

void InspectorClient::SetAmplitude(bool amplitude) { is_amplitude = amplitude; }

void InspectorClient::GetFrameSize(int &width, int &height) {
  width = this->width;
  height = this->height;
}

void InspectorClient::SetFrameSize(int width, int height) {
  this->width = width;
  this->height = height;
}