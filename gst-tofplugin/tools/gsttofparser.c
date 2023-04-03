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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gsttofparser
 *
 * The tofparser element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! tofparser ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbaseparse.h>
#include "gsttofparser.h"

GST_DEBUG_CATEGORY_STATIC (gst_tofparser_debug_category);
#define GST_CAT_DEFAULT gst_tofparser_debug_category

/* prototypes */


static void gst_tofparser_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_tofparser_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_tofparser_dispose (GObject * object);
static void gst_tofparser_finalize (GObject * object);

static gboolean gst_tofparser_start (GstBaseParse * parse);
static gboolean gst_tofparser_stop (GstBaseParse * parse);
static gboolean gst_tofparser_set_sink_caps (GstBaseParse * parse,
    GstCaps * caps);
static GstFlowReturn gst_tofparser_handle_frame (GstBaseParse * parse,
    GstBaseParseFrame * frame, gint * skipsize);
static GstFlowReturn gst_tofparser_pre_push_frame (GstBaseParse * parse,
    GstBaseParseFrame * frame);
static gboolean gst_tofparser_convert (GstBaseParse * parse, GstFormat src_format,
    gint64 src_value, GstFormat dest_format, gint64 * dest_value);
static gboolean gst_tofparser_sink_event (GstBaseParse * parse, GstEvent * event);
static gboolean gst_tofparser_src_event (GstBaseParse * parse, GstEvent * event);
static GstCaps *gst_tofparser_get_sink_caps (GstBaseParse * parse,
    GstCaps * filter);
static GstFlowReturn gst_tofparser_detect (GstBaseParse * parse,
    GstBuffer * buffer);

enum
{
  PROP_0
};

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
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_tofparser_set_property;
  gobject_class->get_property = gst_tofparser_get_property;
  gobject_class->dispose = gst_tofparser_dispose;
  gobject_class->finalize = gst_tofparser_finalize;
  base_parse_class->start = GST_DEBUG_FUNCPTR (gst_tofparser_start);
  base_parse_class->stop = GST_DEBUG_FUNCPTR (gst_tofparser_stop);
  base_parse_class->set_sink_caps = GST_DEBUG_FUNCPTR (gst_tofparser_set_sink_caps);
  base_parse_class->handle_frame = GST_DEBUG_FUNCPTR (gst_tofparser_handle_frame);
  base_parse_class->pre_push_frame = GST_DEBUG_FUNCPTR (gst_tofparser_pre_push_frame);
  base_parse_class->convert = GST_DEBUG_FUNCPTR (gst_tofparser_convert);
  base_parse_class->sink_event = GST_DEBUG_FUNCPTR (gst_tofparser_sink_event);
  base_parse_class->src_event = GST_DEBUG_FUNCPTR (gst_tofparser_src_event);
  base_parse_class->get_sink_caps = GST_DEBUG_FUNCPTR (gst_tofparser_get_sink_caps);
  base_parse_class->detect = GST_DEBUG_FUNCPTR (gst_tofparser_detect);

}

static void
gst_tofparser_init (GstTofparser *tofparser)
{
}

void
gst_tofparser_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstTofparser *tofparser = GST_TOFPARSER (object);

  GST_DEBUG_OBJECT (tofparser, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_tofparser_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstTofparser *tofparser = GST_TOFPARSER (object);

  GST_DEBUG_OBJECT (tofparser, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
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

  return TRUE;
}

static gboolean
gst_tofparser_stop (GstBaseParse * parse)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "stop");

  return TRUE;
}

static gboolean
gst_tofparser_set_sink_caps (GstBaseParse * parse, GstCaps * caps)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "set_sink_caps");

  return TRUE;
}

static GstFlowReturn
gst_tofparser_handle_frame (GstBaseParse * parse, GstBaseParseFrame * frame,
    gint * skipsize)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "handle_frame");

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_tofparser_pre_push_frame (GstBaseParse * parse, GstBaseParseFrame * frame)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "pre_push_frame");

  return GST_FLOW_OK;
}

static gboolean
gst_tofparser_convert (GstBaseParse * parse, GstFormat src_format,
    gint64 src_value, GstFormat dest_format, gint64 * dest_value)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "convert");

  return TRUE;
}

static gboolean
gst_tofparser_sink_event (GstBaseParse * parse, GstEvent * event)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "sink_event");

  return TRUE;
}

static gboolean
gst_tofparser_src_event (GstBaseParse * parse, GstEvent * event)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "src_event");

  return TRUE;
}

static GstCaps *
gst_tofparser_get_sink_caps (GstBaseParse * parse, GstCaps * filter)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "get_sink_caps");

  return NULL;
}

static GstFlowReturn
gst_tofparser_detect (GstBaseParse * parse, GstBuffer * buffer)
{
  GstTofparser *tofparser = GST_TOFPARSER (parse);

  GST_DEBUG_OBJECT (tofparser, "detect");

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "tofparser", GST_RANK_NONE,
      GST_TYPE_TOFPARSER);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    tofparser,
    "FIXME plugin description",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

