/* GStreamer
 * Copyright (C) 2023 Deep In Sight <lnlinh93@dinsight.ai>
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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __TOFMETA_H__
#define __TOFMETA_H__

#include <glib.h>
#include <gst/gst.h>

#define META_TOF_ADD(b) ((GstMetaTof*)gst_buffer_add_meta(b, gst_meta_tof_get_info(), NULL))
#define META_TOF_GET(b) ((GstMetaTof*)gst_buffer_get_meta(b, gst_meta_tof_api_get_type()))

typedef struct _GstMetaTof GstMetaTof;

struct _GstMetaTof {
  GstMeta meta;
  // sensor meta
  guint32 width;
  guint32 height;
  guint32 modulation_frequency;
  // lens meta
  // gfloat cx;
  // gfloat cy;
  // gfloat fx;
  // gfloat fy;
  // ...
};

GType gst_meta_tof_api_get_type (void);
const GstMetaInfo * gst_meta_tof_get_info (void);

#endif