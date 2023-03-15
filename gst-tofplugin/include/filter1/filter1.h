/* GStreamer
 * Copyright (C) 2023 FIXME <fixme@example.com>
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

#ifndef _GST_FILTER1_H_
#define _GST_FILTER1_H_

#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_FILTER1   (gst_filter1_get_type())
#define GST_FILTER1(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_FILTER1,GstFilter1))
#define GST_FILTER1_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_FILTER1,GstFilter1Class))
#define GST_IS_FILTER1(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_FILTER1))
#define GST_IS_FILTER1_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_FILTER1))

typedef struct _GstFilter1 GstFilter1;
typedef struct _GstFilter1Class GstFilter1Class;

struct _GstFilter1
{
  GstBaseTransform base_filter1;

};

struct _GstFilter1Class
{
  GstBaseTransformClass base_filter1_class;
};

GType gst_filter1_get_type (void);

G_END_DECLS

#endif
