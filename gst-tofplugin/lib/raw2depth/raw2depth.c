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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-raw2depth
 *
 * The raw2depth element take a raw tof frame, and calculate depth and
 * amplitude map. Raw frame contains multiple subframes, called phases.
 * Each subframe corresponds to one metadata attach to the raw frame,
 * and metadata of each subframe shows which phase it was captured at.
 *
 * TODO: add phase information in metadata and calculate accordingly.
 * For now, the phase order is 0-180-90-270
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v filesrc location=./test.ek640raw !
 *  tofparse ! raw2depth ! fakesink
 * ]|
 * This pipeline read raw data from local file, parse it, convert raw
 * to depth/amplitude, then discard the result
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/base/gstbasetransform.h>
#include <gst/gst.h>
#include <lib/raw2depth/raw2depth.h>

GST_DEBUG_CATEGORY_STATIC(gst_raw2depth_debug_category);
#define GST_CAT_DEFAULT gst_raw2depth_debug_category

/* prototypes */

// static void gst_raw2depth_set_property (GObject * object,
//     guint property_id, const GValue * value, GParamSpec * pspec);
// static void gst_raw2depth_get_property (GObject * object,
//     guint property_id, GValue * value, GParamSpec * pspec);
static void gst_raw2depth_dispose(GObject *object);
static void gst_raw2depth_finalize(GObject *object);

static GstCaps *gst_raw2depth_transform_caps(GstBaseTransform *trans,
                                             GstPadDirection direction,
                                             GstCaps *caps, GstCaps *filter);

static gboolean gst_raw2depth_get_unit_size(GstBaseTransform *trans,
                                            GstCaps *caps, gsize *size);
// static gboolean gst_raw2depth_start(GstBaseTransform *trans);
// static gboolean gst_raw2depth_stop(GstBaseTransform *trans);

// static gboolean gst_raw2depth_copy_metadata(GstBaseTransform *trans,
//                                             GstBuffer *input,
//                                             GstBuffer *outbuf);
// static gboolean gst_raw2depth_transform_meta(GstBaseTransform *trans,
//                                              GstBuffer *outbuf, GstMeta
//                                              *meta, GstBuffer *inbuf);
static GstFlowReturn gst_raw2depth_transform(GstBaseTransform *trans,
                                             GstBuffer *inbuf,
                                             GstBuffer *outbuf);

enum { PROP_0 };

/* pad templates */

static GstStaticPadTemplate gst_raw2depth_src_template =
    GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("video/tof, "
                                            "format=(string){DA_F32}, "
                                            "width=(int)[1,1920], "
                                            "height=(int)[1,1080], "
                                            "framerate=(fraction)[0/1, "
                                            "30/1]"));

static GstStaticPadTemplate gst_raw2depth_sink_template =
    GST_STATIC_PAD_TEMPLATE("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("video/tof, "
                                            "format=(string){ek640raw}, "
                                            "pixel_size=(int)2, "
                                            "num_subframes=(int){2,4,8}, "
                                            "width=(int)[1,1920], "
                                            "height=(int)[1,1080], "
                                            "framerate=(fraction)[0/1, "
                                            "30/1]"));

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(
    GstRaw2depth, gst_raw2depth, GST_TYPE_BASE_TRANSFORM,
    GST_DEBUG_CATEGORY_INIT(gst_raw2depth_debug_category, "raw2depth", 0,
                            "debug category for raw2depth element"));

static void gst_raw2depth_class_init(GstRaw2depthClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                            &gst_raw2depth_src_template);
  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                            &gst_raw2depth_sink_template);

  gst_element_class_set_static_metadata(
      GST_ELEMENT_CLASS(klass), "FIXME Long name", "Generic",
      "FIXME Description", "FIXME <fixme@example.com>");

  // gobject_class->set_property = gst_raw2depth_set_property;
  // gobject_class->get_property = gst_raw2depth_get_property;
  gobject_class->dispose = gst_raw2depth_dispose;
  gobject_class->finalize = gst_raw2depth_finalize;
  base_transform_class->transform_caps =
      GST_DEBUG_FUNCPTR(gst_raw2depth_transform_caps);

  base_transform_class->get_unit_size =
      GST_DEBUG_FUNCPTR(gst_raw2depth_get_unit_size);
  // base_transform_class->start = GST_DEBUG_FUNCPTR(gst_raw2depth_start);
  // base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_raw2depth_stop);

  // base_transform_class->copy_metadata =
  //     GST_DEBUG_FUNCPTR(gst_raw2depth_copy_metadata);
  // base_transform_class->transform_meta =
  //     GST_DEBUG_FUNCPTR(gst_raw2depth_transform_meta);

  base_transform_class->transform = GST_DEBUG_FUNCPTR(gst_raw2depth_transform);
}

static void gst_raw2depth_init(GstRaw2depth *raw2depth) {}

// void gst_raw2depth_set_property(GObject *object, guint property_id,
//                                 const GValue *value, GParamSpec *pspec) {
//   GstRaw2depth *raw2depth = GST_RAW2DEPTH(object);

//   GST_DEBUG_OBJECT(raw2depth, "set_property");

//   switch (property_id) {
//     default:
//       G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
//       break;
//   }
// }

// void gst_raw2depth_get_property(GObject *object, guint property_id,
//                                 GValue *value, GParamSpec *pspec) {
//   GstRaw2depth *raw2depth = GST_RAW2DEPTH(object);

//   GST_DEBUG_OBJECT(raw2depth, "get_property");

//   switch (property_id) {
//     default:
//       G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
//       break;
//   }
// }

void gst_raw2depth_dispose(GObject *object) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(object);

  GST_DEBUG_OBJECT(raw2depth, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS(gst_raw2depth_parent_class)->dispose(object);
}

void gst_raw2depth_finalize(GObject *object) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(object);

  GST_DEBUG_OBJECT(raw2depth, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS(gst_raw2depth_parent_class)->finalize(object);
}

static GstCaps *gst_raw2depth_transform_caps(GstBaseTransform *trans,
                                             GstPadDirection direction,
                                             GstCaps *caps, GstCaps *filter) {
  GstRaw2depth *raw2depth __attribute__((unused)) = GST_RAW2DEPTH(trans);
  GstCaps *othercaps;

  othercaps = gst_caps_copy(caps);

  /* Copy other caps and modify as appropriate */
  /* This works for the simplest cases, where the transform modifies one
   * or more fields in the caps structure.  It does not work correctly
   * if passthrough caps are preferred. */
  if (direction == GST_PAD_SRC) {
    /* transform caps going upstream */
    /* simply remove the format and use template's formats */
    GstStructure *othercaps_struct = gst_caps_get_structure(othercaps, 0);
    gst_structure_remove_field(othercaps_struct, "format");
  } else {
    /* transform caps going downstream */
    /* remove num_subframes and pixel_size */
    GstStructure *othercaps_struct = gst_caps_get_structure(othercaps, 0);
    gst_structure_remove_field(othercaps_struct, "format");
    gst_structure_remove_field(othercaps_struct, "num_subframes");
    gst_structure_remove_field(othercaps_struct, "pixel_size");
  }

  if (filter) {
    GstCaps *intersect;

    intersect = gst_caps_intersect(othercaps, filter);
    gst_caps_unref(othercaps);

    return intersect;
  } else {
    return othercaps;
  }
}

static gboolean gst_raw2depth_get_unit_size(GstBaseTransform *trans,
                                            GstCaps *caps, gsize *size) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);
  const GstStructure *caps_struct;
  const gchar *fmt;
  gint width, height, pixel_size, num_subframes;

  GST_DEBUG_OBJECT(raw2depth, "get_unit_size");

  if (gst_caps_get_size(caps) != 1) {
    goto fail;
  }

  caps_struct = gst_caps_get_structure(caps, 0);
  if (!caps_struct) {
    goto fail;
  }

  if (!gst_structure_has_field(caps_struct, "format") ||
      !gst_structure_has_field(caps_struct, "width") ||
      !gst_structure_has_field(caps_struct, "height")) {
    goto fail;
  }

  fmt = gst_structure_get_string(caps_struct, "format");
  gst_structure_get_int(caps_struct, "width", &width);
  gst_structure_get_int(caps_struct, "height", &height);

  if (g_strcmp0(fmt, "ek640raw") == 0) {
    gst_structure_get_int(caps_struct, "pixel_size", &pixel_size);
    gst_structure_get_int(caps_struct, "num_subframes", &num_subframes);
    *size = width * height * pixel_size * num_subframes;
  } else if (g_strcmp0(fmt, "DA_F32") == 0) {
    *size = width * height * sizeof(gfloat) * 2;
  } else if (g_strcmp0(fmt, "D_F32") == 0) {
    *size = width * height * sizeof(gfloat);
  } else {
    goto fail;
  }

  GST_DEBUG_OBJECT(raw2depth,
                   "format: %s, width: %d, height: %d, "
                   "size: %" G_GSIZE_FORMAT,
                   fmt, width, height, *size);

  return TRUE;
fail:
  return FALSE;
}

/* states */
// static gboolean gst_raw2depth_start(GstBaseTransform *trans) {
//   GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);

//   GST_DEBUG_OBJECT(raw2depth, "start");

//   return TRUE;
// }

// static gboolean gst_raw2depth_stop(GstBaseTransform *trans) {
//   GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);

//   GST_DEBUG_OBJECT(raw2depth, "stop");

//   return TRUE;
// }

/* metadata */
// static gboolean gst_raw2depth_copy_metadata(GstBaseTransform *trans,
//                                             GstBuffer *input,
//                                             GstBuffer *outbuf) {
//   GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);

//   GST_DEBUG_OBJECT(raw2depth, "copy_metadata");

//   return TRUE;
// }

// static gboolean gst_raw2depth_transform_meta(GstBaseTransform *trans,
//                                              GstBuffer *outbuf, GstMeta
//                                              *meta, GstBuffer *inbuf) {
//   GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);

//   GST_DEBUG_OBJECT(raw2depth, "transform_meta");

//   return TRUE;
// }

/* transform */
static GstFlowReturn gst_raw2depth_transform(GstBaseTransform *trans,
                                             GstBuffer *inbuf,
                                             GstBuffer *outbuf) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);

  GST_DEBUG_OBJECT(raw2depth, "transform");

  return GST_FLOW_OK;
}