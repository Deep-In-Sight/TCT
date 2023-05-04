/* GStreamer
 * Copyright (C) 2023 Deep In Sight
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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <lib/common/tofmeta.h>

// get the registered api type
GType gst_meta_tof_api_get_type(void) {
  static GType type;
  static const gchar* tags[] = {NULL};

  if (g_once_init_enter(&type)) {
    GType _type = gst_meta_api_type_register("GstMetaTofApi", tags);
    g_once_init_leave(&type, _type);
  }

  return type;
}

static gboolean gst_meta_tof_init(GstMetaTof* meta, gpointer params,
                                  GstBuffer* buf) {
  guint32* tmp;

  if (params == NULL) {
    goto init_done;
  }

  tmp = (guint32*)params;
  meta->modulation_frequency = *tmp++;
  meta->sensor_temperature = *tmp++;
  meta->rngchk_low = *tmp++;
  meta->rngchk_high = *tmp;

init_done:
  return TRUE;
}

// get the actual implementation of the api
const GstMetaInfo* gst_meta_tof_get_info(void) {
  static const GstMetaInfo* meta_info = NULL;

  if (g_once_init_enter(&meta_info)) {
    const GstMetaInfo* _meta_info = gst_meta_register(
        gst_meta_tof_api_get_type(), "GstMetaTof", sizeof(GstMetaTof),
        (GstMetaInitFunction)gst_meta_tof_init, (GstMetaFreeFunction)NULL,
        (GstMetaTransformFunction)NULL);
    g_once_init_leave(&meta_info, _meta_info);
  }

  return meta_info;
}