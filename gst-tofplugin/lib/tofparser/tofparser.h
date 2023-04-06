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

#ifndef _GST_TOFPARSER_H_
#define _GST_TOFPARSER_H_

#include <gst/base/gstbaseparse.h>

G_BEGIN_DECLS

#define GST_TYPE_TOFPARSER   (gst_tofparser_get_type())
#define GST_TOFPARSER(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TOFPARSER,GstTofparser))
#define GST_TOFPARSER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TOFPARSER,GstTofparserClass))
#define GST_IS_TOFPARSER(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TOFPARSER))
#define GST_IS_TOFPARSER_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TOFPARSER))

typedef struct _GstTofparser GstTofparser;
typedef struct _GstTofparserClass GstTofparserClass;
typedef struct _StreamHeader StreamHeader;

struct _StreamHeader {
  guint32 container_header_size;
  guint32 subframe_header_size;
  guint32 frame_width;
  guint32 frame_height;
  guint32 framerate_num;
  guint32 framerate_den;
  guint32 pixel_size;
  guint32 num_subframes;
  guint32 num_frames;
  // guint32 others;
};

struct _GstTofparser
{
  GstBaseParse base_tofparser;
  gboolean is_first_frame;
  
  //stream format
  StreamHeader sh;
};

struct _GstTofparserClass
{
  GstBaseParseClass base_tofparser_class;
};

GType gst_tofparser_get_type (void);

G_END_DECLS

#endif
