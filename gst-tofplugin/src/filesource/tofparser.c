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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gsttofparser
 *
 * The tofparser element receives (small) buffers from upstream (likely 
 * a filesrc), scan for file header and (or) frame headers, then output 
 * (big) frame buffers to donwstream. It also attach some metadata to 
 * each frame buffer.
 * 
 * TODO: actually parse file header and frame header
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 filesrc location=/dev/random blocksize=65536 num-buffers=100 \
 *  ! tofparser ! filesink location=./testdatasink
 * ]|
 * The above pipeline read random data, send to tofparser. The parser read the 
 * the buffers, remove headers and save the extracted frames into a file.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbaseparse.h>
#include "filesource/tofparser.h"

GST_DEBUG_CATEGORY_STATIC (gst_tofparser_debug_category);
#define GST_CAT_DEFAULT gst_tofparser_debug_category

/* prototypes */
static void gst_tofparser_dispose (GObject * object);
static void gst_tofparser_finalize (GObject * object);
static gboolean gst_tofparser_start (GstBaseParse * parse);
static gboolean gst_tofparser_stop (GstBaseParse * parse);
static GstFlowReturn gst_tofparser_handle_frame (GstBaseParse * parse,
    GstBaseParseFrame * frame, gint * skipsize);
static GstFlowReturn gst_tofparser_detect (GstBaseParse * parse,
    GstBuffer * buffer);
void tofparser_parse_file_header(GstBaseParse* parse, GstBuffer* buffer);

enum
{
  PROP_0
};

// #define FRAME_SIZE (640*480*2*4)
// #define HEADER_SIZE (512)
#define FRAME_SIZE (640*480*2*4)
#define HEADER_SIZE (16)
#define FILEHEADER_SIZE (64)

/* pad templates */

static GstStaticPadTemplate gst_tofparser_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/unknown")
    );

static GstStaticPadTemplate gst_tofparser_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/unknown")
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTofparser, gst_tofparser, GST_TYPE_BASE_PARSE,
  GST_DEBUG_CATEGORY_INIT (gst_tofparser_debug_category, "tofparser", 0,
  "debug category for tofparser element"));

static void
gst_tofparser_class_init (GstTofparserClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseParseClass *base_parse_class = GST_BASE_PARSE_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_tofparser_src_template);
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_tofparser_sink_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "tofparser", "Generic", "Parse a stream of tof data. It reads a ",
      "FIXME <fixme@example.com>");

  gobject_class->dispose = gst_tofparser_dispose;
  gobject_class->finalize = gst_tofparser_finalize;
  base_parse_class->start = GST_DEBUG_FUNCPTR (gst_tofparser_start);
  base_parse_class->stop = GST_DEBUG_FUNCPTR (gst_tofparser_stop);
  base_parse_class->handle_frame = GST_DEBUG_FUNCPTR (gst_tofparser_handle_frame);
  // base_parse_class->detect = GST_DEBUG_FUNCPTR (gst_tofparser_detect);

}

static void
gst_tofparser_init (GstTofparser *tofparser)
{
  tofparser->is_first_frame = TRUE;
  tofparser->video_type = -1;
  tofparser->file_header_size = -1;
  tofparser->header_size = -1;
  tofparser->frame_size = -1;
}

void
gst_tofparser_dispose (GObject * object)
{
  GstTofparser *tofparser = GST_TOFPARSER (object);

  GST_DEBUG_OBJECT (tofparser, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_tofparser_parent_class)->dispose (object);
}

void
gst_tofparser_finalize (GObject * object)
{
  GstTofparser *tofparser = GST_TOFPARSER (object);

  GST_DEBUG_OBJECT (tofparser, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_tofparser_parent_class)->finalize (object);
}

static gboolean
gst_tofparser_start (GstBaseParse * parse)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "start");

  gst_base_parse_set_min_frame_size(parse, FILEHEADER_SIZE);

  return TRUE;
}

static gboolean
gst_tofparser_stop (GstBaseParse * parse)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "stop");

  return TRUE;
}

static GstFlowReturn
gst_tofparser_handle_frame (GstBaseParse * parse, GstBaseParseFrame * frame,
    gint * skipsize)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);
  gssize flush_size, buffer_size, buffer_offset;

  buffer_size = gst_buffer_get_size(frame->buffer);
  buffer_offset = GST_BUFFER_OFFSET(frame->buffer);

  GST_DEBUG_OBJECT (tofparser, 
    "got buffer with size %" G_GSIZE_FORMAT 
    ", offset %" G_GSIZE_FORMAT,
    buffer_size,
    buffer_offset);
  
  if (tofparser->is_first_frame) {
    tofparser->is_first_frame = FALSE;
    tofparser_parse_file_header(parse, frame->buffer);
    *skipsize = FILEHEADER_SIZE;
  } else {
    *skipsize = 0;
  }

  flush_size = HEADER_SIZE + FRAME_SIZE;
  gst_base_parse_set_min_frame_size(parse, flush_size);

  //first frame only guaranteed to has complete file header
  if (buffer_size < flush_size) {
    return GST_FLOW_OK;
  }
  
  // simply remove header and transfer frame to downstream
  // parse the frame header and add meta to the frame->out_buffer later
  frame->out_buffer = gst_buffer_copy_region(frame->buffer, 
    GST_BUFFER_COPY_MEMORY, HEADER_SIZE, FRAME_SIZE);
  // GST_BUFFER_PTS(frame->out_buffer) = next_pts(tofparser);
  // gst_buffer_add_meta(fram->out_buffer, info, params);

  gst_base_parse_finish_frame(parse, frame, flush_size);
  
  return GST_FLOW_OK;
}

static GstFlowReturn
gst_tofparser_detect (GstBaseParse * parse, GstBuffer * buffer)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "detect");

  // pretend to read the first buffer and setup the video type here
  tofparser->video_type = 1;
  tofparser->frame_size = FRAME_SIZE;
  tofparser->header_size = HEADER_SIZE;
  tofparser->file_header_size = FILEHEADER_SIZE;
  tofparser->is_first_frame = TRUE;

  return GST_FLOW_OK;
}

void tofparser_parse_file_header(GstBaseParse* parse, GstBuffer* buffer) {
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  //hard code for now, parse file header buffer later
  tofparser->video_type = 1;
  tofparser->frame_size = FRAME_SIZE;
  tofparser->header_size = HEADER_SIZE;
  tofparser->file_header_size = FILEHEADER_SIZE;
  
  GstCaps* src_caps = gst_caps_from_string("something");
  gst_pad_set_caps(GST_BASE_PARSE_SRC_PAD(parse), src_caps);
  gst_caps_unref(src_caps);
}
