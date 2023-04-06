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
 * The tofparser element receives buffers from upstream (a filesrc),
 * scan for file header and (or) frame headers, then output frame
 * buffers to donwstream.
 *
 * The ToF sensor raw data file format is as follow:
 *         ------------------------------------------------
 *  File:  | file_header | frame0 | frame1 | ... | frameN |
 *         ------------------------------------------------
 *                   ___/          \_________________
 *                  /                                \
 *                 -----------------------------------
 *  Depth Frame:   |subframe0|subframe1|...|subframeK|  (K=1/3/7)
 *                 -----------------------------------
 *                        __/           \_________________
 *                       /                                \
 *  Sub- depth frame     ----------------------------------
 * (phase frame):        |subframe_header| pixel data     |
 *                       ----------------------------------
 *
 * File header contains stream infomation: frame size, frame rate. These
 * go to the caps string
 *
 * Subframe header contains sensor information: pixel range check, sensor
 * temperature, etc. These go to the frame buffer metadatas.
 *
 * The tofparser produces GstBuffer with following content:
 *  Frame Buffer
 *  |--memory:
 *  |      |--subframe0 pixeldata
 *  |      |--subframe1 pixeldata
 *  |      |--...
 *  |      |--subframeK pixeldata
 *  |--meta0: parse from subframe0 header
 *  |--meta1: parse from subframe1 header
 *  |--..
 *  |--metaK: parse from subframeK header
 *
 * TODO: design a good header semantic.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 filesrc location=video1.ek640raw ! tofparser ! fakesink
 * ]|
 * The above pipeline read from raw file, parse it, then discard the buffers.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/base/gstbaseparse.h>
#include <gst/gst.h>
#include <lib/common/tofmeta.h>
#include <lib/tofparser/tofparser.h>

GST_DEBUG_CATEGORY_STATIC(gst_tofparser_debug_category);
#define GST_CAT_DEFAULT gst_tofparser_debug_category

/* prototypes */
static void gst_tofparser_dispose(GObject* object);
static void gst_tofparser_finalize(GObject* object);
static gboolean gst_tofparser_start(GstBaseParse* parse);
static gboolean gst_tofparser_stop(GstBaseParse* parse);
static GstFlowReturn gst_tofparser_handle_frame(GstBaseParse* parse,
                                                GstBaseParseFrame* frame,
                                                gint* skipsize);
void tofparser_parse_file_header(GstBaseParse* parse, GstBuffer* buffer);
void tofparser_parse_frame_header(GstBaseParse* parse, GstBuffer* buffer,
                                  GstMetaTof* meta);

enum { PROP_0 };

#define FILEHEADER_SIZE_MAX (1 << 10)

/* pad templates */

static GstStaticPadTemplate gst_tofparser_src_template =
    GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("application/unknown"));

static GstStaticPadTemplate gst_tofparser_sink_template =
    GST_STATIC_PAD_TEMPLATE("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("application/unknown"));

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(
    GstTofparser, gst_tofparser, GST_TYPE_BASE_PARSE,
    GST_DEBUG_CATEGORY_INIT(gst_tofparser_debug_category, "tofparser", 0,
                            "debug category for tofparser element"));

static void gst_tofparser_class_init(GstTofparserClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  GstBaseParseClass* base_parse_class = GST_BASE_PARSE_CLASS(klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                            &gst_tofparser_src_template);
  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                            &gst_tofparser_sink_template);

  gst_element_class_set_static_metadata(
      GST_ELEMENT_CLASS(klass), "tofparser", "Generic",
      "Parse a stream of tof data, remove header "
      "and push frame buffer downstream",
      "Le Ngoc Linh <lnlinh93@dinsight.ai>");

  gobject_class->dispose = gst_tofparser_dispose;
  gobject_class->finalize = gst_tofparser_finalize;
  base_parse_class->start = GST_DEBUG_FUNCPTR(gst_tofparser_start);
  base_parse_class->stop = GST_DEBUG_FUNCPTR(gst_tofparser_stop);
  base_parse_class->handle_frame =
      GST_DEBUG_FUNCPTR(gst_tofparser_handle_frame);
  // base_parse_class->detect = GST_DEBUG_FUNCPTR (gst_tofparser_detect);
}

static void gst_tofparser_init(GstTofparser* tofparser) {
  tofparser->is_first_frame = TRUE;
  tofparser->sh.container_header_size = 0;
  tofparser->sh.subframe_header_size = 0;
  tofparser->sh.frame_width = 0;
  tofparser->sh.frame_height = 0;
  tofparser->sh.framerate_num = 0;
  tofparser->sh.framerate_den = 0;
  tofparser->sh.pixel_size = 0;
  tofparser->sh.num_subframes = 0;
  tofparser->sh.num_frames = 0;
}

void gst_tofparser_dispose(GObject* object) {
  GstTofparser* tofparser = GST_TOFPARSER(object);

  GST_DEBUG_OBJECT(tofparser, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS(gst_tofparser_parent_class)->dispose(object);
}

void gst_tofparser_finalize(GObject* object) {
  GstTofparser* tofparser = GST_TOFPARSER(object);

  GST_DEBUG_OBJECT(tofparser, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS(gst_tofparser_parent_class)->finalize(object);
}

static gboolean gst_tofparser_start(GstBaseParse* parse) {
  GstTofparser* tofparser = GST_TOFPARSER(parse);

  GST_DEBUG_OBJECT(tofparser, "start");

  gst_base_parse_set_min_frame_size(parse, FILEHEADER_SIZE_MAX);

  return TRUE;
}

static gboolean gst_tofparser_stop(GstBaseParse* parse) {
  GstTofparser* tofparser = GST_TOFPARSER(parse);

  GST_DEBUG_OBJECT(tofparser, "stop");

  return TRUE;
}

static GstFlowReturn gst_tofparser_handle_frame(GstBaseParse* parse,
                                                GstBaseParseFrame* frame,
                                                gint* skipsize) {
  GstTofparser* tofparser = GST_TOFPARSER(parse);
  gsize buffer_size, buffer_offset;
  gsize df_size;
  gsize sf_header_offset, sf_header_size;
  gsize sf_payload_offset, sf_payload_size;
  gsize sf_size;
  StreamHeader* sh;
  GstMapInfo mapinfo;
  guint8* data;
  GstMetaTof* meta;

  buffer_size = gst_buffer_get_size(frame->buffer);
  buffer_offset = GST_BUFFER_OFFSET(frame->buffer);

  GST_DEBUG_OBJECT(tofparser,
                   "got buffer with size %" G_GSIZE_FORMAT
                   ", offset %" G_GSIZE_FORMAT,
                   buffer_size, buffer_offset);

  sh = &(tofparser->sh);

  if (tofparser->is_first_frame) {
    tofparser->is_first_frame = FALSE;
    tofparser_parse_file_header(parse, frame->buffer);
    gst_base_parse_set_frame_rate(parse, tofparser->sh.framerate_num,
                                  tofparser->sh.framerate_den, 0, 0);
    /*TODO: set caps from the parsed data above*/
    GstCaps* src_caps = gst_caps_from_string("something");
    gst_pad_set_caps(GST_BASE_PARSE_SRC_PAD(parse), src_caps);
    gst_caps_unref(src_caps);
    *skipsize = sh->container_header_size;
  } else {
    *skipsize = 0;
  }

  sf_header_size = sh->subframe_header_size;
  sf_payload_size = sh->frame_width * sh->frame_height * sh->pixel_size;
  sf_size = sf_header_size + sf_payload_size;
  df_size = sh->num_subframes * sf_size;

  // next time only handle full depth frame buffer
  gst_base_parse_set_min_frame_size(parse, df_size);

  // first frame only guaranteed to has complete file header
  if (buffer_size < df_size) {
    return GST_FLOW_OK;
  }

  gst_buffer_map(frame->buffer, &mapinfo, GST_MAP_READ);
  data = mapinfo.data;
  frame->out_buffer = gst_buffer_new();
  for (int sf = 0; sf < sh->num_subframes; sf++) {
    sf_header_offset = sf * sf_size;
    sf_payload_offset = sf_header_offset + sf_header_size;
    // gstreamer's magic: no new memory allocated
    gst_buffer_copy_into(frame->out_buffer, frame->buffer,
                         GST_BUFFER_COPY_MEMORY, sf_payload_offset,
                         sf_payload_size);

    guint8* sf_header = data + sf_header_offset;
    meta = META_TOF_ADD_PARAMS(frame->out_buffer, (gpointer)sf_header);
    GST_DEBUG_OBJECT(tofparser, "meta initialized: %d %d %d %d",
                     meta->modulation_frequency, meta->sensor_temperature,
                     meta->rngchk_low, meta->rngchk_high);
  }

  gst_buffer_unmap(frame->buffer, &mapinfo);

  GST_DEBUG_OBJECT(tofparser,
                   "sending off out_buffer with size %" G_GSIZE_FORMAT,
                   gst_buffer_get_size(frame->out_buffer));

  gst_base_parse_finish_frame(parse, frame, df_size);

  return GST_FLOW_OK;
}

guint32 parse_next_guint32(guint8** data) {
  guint32* tmp = (guint32*)(*data);
  *data += sizeof(guint32);
  return *tmp;
}

void tofparser_parse_file_header(GstBaseParse* parse, GstBuffer* buffer) {
  GstTofparser* tofparser = GST_TOFPARSER(parse);
  GstMapInfo mapinfo;
  guint8* data;

  gst_buffer_map(buffer, &mapinfo, GST_MAP_READ);

  data = mapinfo.data;
  tofparser->sh.container_header_size = parse_next_guint32(&data);
  tofparser->sh.subframe_header_size = parse_next_guint32(&data);
  tofparser->sh.frame_width = parse_next_guint32(&data);
  tofparser->sh.frame_height = parse_next_guint32(&data);
  tofparser->sh.framerate_num = parse_next_guint32(&data);
  tofparser->sh.framerate_den = parse_next_guint32(&data);
  tofparser->sh.pixel_size = parse_next_guint32(&data);
  tofparser->sh.num_subframes = parse_next_guint32(&data);
  tofparser->sh.num_frames = parse_next_guint32(&data);

  gst_buffer_unmap(buffer, &mapinfo);
}