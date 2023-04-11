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

#ifndef _GST_RAW2DEPTH_H_
#define _GST_RAW2DEPTH_H_

#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_RAW2DEPTH (gst_raw2depth_get_type())
#define GST_RAW2DEPTH(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_RAW2DEPTH, GstRaw2depth))
#define GST_RAW2DEPTH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_RAW2DEPTH, GstRaw2depthClass))
#define GST_IS_RAW2DEPTH(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_RAW2DEPTH))
#define GST_IS_RAW2DEPTH_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_RAW2DEPTH))

typedef struct _GstRaw2depth GstRaw2depth;
typedef struct _GstRaw2depthClass GstRaw2depthClass;

struct _GstRaw2depth {
  GstBaseTransform base_raw2depth;

  GstBufferPool *pool;
};

struct _GstRaw2depthClass {
  GstBaseTransformClass base_raw2depth_class;
};

GType gst_raw2depth_get_type(void);

G_END_DECLS

#endif
