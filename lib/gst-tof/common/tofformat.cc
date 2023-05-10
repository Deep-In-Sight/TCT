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

#include <gst-tof/common/tofformat.h>

const gchar* raw_fmts_str[] = {"ek640raw"};
int num_raw_fmts = sizeof(raw_fmts_str) / sizeof(raw_fmts_str[0]);

const gchar* depth_fmts_str[] = {
    // "D_F16",
    "D_F32",
    // "DA_F16",
    "DA_F32"};
int num_depth_fmts = sizeof(depth_fmts_str) / sizeof(depth_fmts_str[0]);

GValue* tofformat_get_raw_fmts(void) {
  static GValue* va = NULL;

  if (g_once_init_enter(&va)) {
    static GValue va_tmp = G_VALUE_INIT;
    g_value_init(&va_tmp, GST_TYPE_LIST);

    for (int fmt = 0; fmt < num_raw_fmts; fmt++) {
      GValue v = G_VALUE_INIT;
      g_value_init(&v, G_TYPE_STRING);
      g_value_set_string(&v, raw_fmts_str[fmt]);
      gst_value_list_append_and_take_value(&va_tmp, &v);
    }

    g_once_init_leave(&va, &va_tmp);
  }

  return va;
}

GValue* tofformat_get_depth_fmts(void) {
  static GValue* va = NULL;

  if (g_once_init_enter(&va)) {
    static GValue va_tmp = G_VALUE_INIT;
    g_value_init(&va_tmp, GST_TYPE_LIST);

    for (int fmt = 0; fmt < num_depth_fmts; fmt++) {
      GValue v = G_VALUE_INIT;
      g_value_init(&v, G_TYPE_STRING);
      g_value_set_string(&v, depth_fmts_str[fmt]);
      gst_value_list_append_and_take_value(&va_tmp, &v);
    }

    g_once_init_leave(&va, &va_tmp);
  }

  return va;
}

gsize tofformat_get_num_raw_fmts(void) { return num_raw_fmts; }

gsize tofformat_get_num_depth_fmts(void) { return num_depth_fmts; }