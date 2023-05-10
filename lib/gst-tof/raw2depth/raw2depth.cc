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

#include <gst-tof/common/tofmeta.h>
#include <gst-tof/raw2depth/raw2depth.h>
#include <gst/base/gstbasetransform.h>
#include <gst/gst.h>
#include <math.h>

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
static gboolean gst_raw2depth_set_caps(GstBaseTransform *trans, GstCaps *incaps,
                                       GstCaps *outcaps);
gboolean get_output_buffer_size(GstBaseTransform *trans, GstCaps *caps,
                                gsize *size);
// static gboolean gst_raw2depth_start(GstBaseTransform *trans);
// static gboolean gst_raw2depth_stop(GstBaseTransform *trans);

// static gboolean gst_raw2depth_copy_metadata(GstBaseTransform *trans,
//                                             GstBuffer *input,
//                                             GstBuffer *outbuf);
// static gboolean gst_raw2depth_transform_meta(GstBaseTransform *trans,
//                                              GstBuffer *outbuf, GstMeta
//                                              *meta, GstBuffer *inbuf);
static GstFlowReturn gst_raw2depth_prepare_output_buffer(
    GstBaseTransform *trans, GstBuffer *inbuf, GstBuffer **outbuf);
static GstFlowReturn gst_raw2depth_transform(GstBaseTransform *trans,
                                             GstBuffer *inbuf,
                                             GstBuffer *outbuf);

enum { PROP_0 };

/* pad templates */

static GstStaticPadTemplate gst_raw2depth_src_template =
    GST_STATIC_PAD_TEMPLATE(
        "src", GST_PAD_SRC, GST_PAD_ALWAYS,
        GST_STATIC_CAPS("video/tof, "
                        "format=(string){DA_F16, DA_F32, D_F16, D_F32}, "
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
      GST_ELEMENT_CLASS(klass), "raw2depth", "Generic",
      "Convert ToF Raw data to Depth and Amplitude data",
      "Le Ngoc Linh <lnlinh93@dinsight.ai>");

  gobject_class->dispose = gst_raw2depth_dispose;
  gobject_class->finalize = gst_raw2depth_finalize;
  base_transform_class->transform_caps =
      GST_DEBUG_FUNCPTR(gst_raw2depth_transform_caps);
  base_transform_class->set_caps = GST_DEBUG_FUNCPTR(gst_raw2depth_set_caps);
  base_transform_class->prepare_output_buffer =
      GST_DEBUG_FUNCPTR(gst_raw2depth_prepare_output_buffer);
  base_transform_class->transform = GST_DEBUG_FUNCPTR(gst_raw2depth_transform);
}

static void gst_raw2depth_init(GstRaw2depth *raw2depth) {
  raw2depth->pool = NULL;
}

void gst_raw2depth_dispose(GObject *object) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(object);
  GstBufferPool *pool = raw2depth->pool;

  GST_DEBUG_OBJECT(raw2depth, "dispose");

  /* clean up as possible.  may be called multiple times */
  if (pool) {
    if (gst_buffer_pool_is_active(pool)) {
      gst_buffer_pool_set_active(pool, FALSE);
    }
    gst_object_unref(pool);
    raw2depth->pool = NULL;
  }

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

static GstFlowReturn gst_raw2depth_ek640raw_to_DA_F32(GstBaseTransform *trans,
                                                      GstBuffer *in,
                                                      GstBuffer *out) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);
  GstMetaTof *meta;
  GstMapInfo map_in, map_out;
  int nr_pixels;
  float depth_scale;
  float mod_freq;

  GST_DEBUG_OBJECT(raw2depth, "gst_raw2depth_ek640raw_to_DA_F32");

  meta = META_TOF_GET(in);
  GST_DEBUG_OBJECT(raw2depth, "found meta at %p", meta);
  mod_freq = meta->modulation_frequency;
  GST_DEBUG_OBJECT(raw2depth, "mod freq = %f", mod_freq);
  nr_pixels = raw2depth->width * raw2depth->height;
  gst_buffer_map(in, &map_in, GST_MAP_READ);
  gst_buffer_map(out, &map_out, GST_MAP_WRITE);

  gshort *phase0, *phase2, *phase1, *phase3;
  gfloat *depth, *amplitude;

  phase0 = (gshort *)map_in.data;
  phase2 = phase0 + nr_pixels;
  phase1 = phase2 + nr_pixels;
  phase3 = phase1 + nr_pixels;

  depth = (gfloat *)map_out.data;
  amplitude = depth + nr_pixels;

  depth_scale = 3e8 / mod_freq / (4 * M_PI);

  for (int pixel = 0; pixel < nr_pixels; pixel++) {
    float Q = phase3[pixel] - phase1[pixel];
    float I = phase2[pixel] - phase0[pixel];
    amplitude[pixel] = 0.5 * sqrt(Q * Q + I * I);
    float phase_diff = M_PI + atan2(Q, I);
    depth[pixel] = depth_scale * phase_diff;
  }

  gst_buffer_unmap(in, &map_in);
  gst_buffer_unmap(out, &map_out);

  return GST_FLOW_OK;
}

/* caps negotiation success, now configure subclass accordingly */
static gboolean gst_raw2depth_set_caps(GstBaseTransform *trans, GstCaps *incaps,
                                       GstCaps *outcaps) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);
  GstStructure *in_struct = gst_caps_get_structure(incaps, 0);
  GstStructure *out_struct = gst_caps_get_structure(outcaps, 0);
  int width, height;

  GST_DEBUG_OBJECT(raw2depth, "set_caps");
  const gchar *sink_fmt = gst_structure_get_string(in_struct, "format");
  const gchar *src_fmt = gst_structure_get_string(out_struct, "format");

  GST_DEBUG_OBJECT(raw2depth, "chosing converter for %s (src) and %s (sink)",
                   src_fmt, sink_fmt);
  if (g_strcmp0(sink_fmt, "ek640raw") == 0 &&
      g_strcmp0(src_fmt, "DA_F32") == 0) {
    raw2depth->convert = gst_raw2depth_ek640raw_to_DA_F32;
  } else {
    GST_DEBUG_OBJECT(raw2depth, "unsupported format");
  }

  gst_structure_get_int(in_struct, "width", &width);
  gst_structure_get_int(in_struct, "height", &height);
  raw2depth->width = width;
  raw2depth->height = height;

  return TRUE;
}

gboolean get_output_buffer_size(GstBaseTransform *trans, GstCaps *caps,
                                gsize *size) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);
  const GstStructure *caps_struct;
  const gchar *fmt;
  gint width, height;

  GST_DEBUG_OBJECT(raw2depth, "get_output_buffer_size");

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

  if (g_strcmp0(fmt, "DA_F32") == 0) {
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

static GstFlowReturn gst_raw2depth_prepare_output_buffer(
    GstBaseTransform *trans, GstBuffer *inbuf, GstBuffer **outbuf) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);
  GstBaseTransformClass *bclass = GST_BASE_TRANSFORM_GET_CLASS(trans);
  GstBufferPool *pool = raw2depth->pool;
  GstStructure *config;
  gsize outsize;
  GstFlowReturn ret;
  GstPad *src_pad;
  GstCaps *src_caps;

  if (!pool) {
    GST_DEBUG_OBJECT(trans, "Creating new buffer pool");
    pool = gst_buffer_pool_new();

    src_pad = GST_BASE_TRANSFORM_SRC_PAD(trans);
    src_caps = gst_pad_get_current_caps(src_pad);
    if (src_caps == NULL) {
      goto no_outcaps;
    }

    config = gst_buffer_pool_get_config(pool);
    if (!get_output_buffer_size(trans, src_caps, &outsize)) {
      GST_DEBUG_OBJECT(trans, "cannot get output buffer size");
      goto config_failed;
    }
    gst_buffer_pool_config_set_params(config, src_caps, outsize, 1, 0);
    if (!gst_buffer_pool_set_config(pool, config)) {
      goto config_failed;
    }
    raw2depth->pool = pool;
  }

  if (!gst_buffer_pool_is_active(pool)) {
    GST_DEBUG_OBJECT(trans, "setting pool %p active", pool);
    if (!gst_buffer_pool_set_active(pool, TRUE)) {
      goto activate_failed;
    }
  }

  GST_DEBUG_OBJECT(raw2depth, "using pool alloc");
  ret = gst_buffer_pool_acquire_buffer(pool, outbuf, NULL);
  if (ret != GST_FLOW_OK) {
    goto alloc_failed;
  }
  GST_DEBUG_OBJECT(raw2depth, "acquired buffer %p with size %" G_GSIZE_FORMAT,
                   *outbuf, gst_buffer_get_size(*outbuf));

  /* copy the metadata */
  if (bclass->copy_metadata) {
    if (!bclass->copy_metadata(trans, inbuf, *outbuf)) {
      /* something failed, post a warning */
      GST_ELEMENT_WARNING(trans, STREAM, NOT_IMPLEMENTED,
                          ("could not copy metadata"), (NULL));
    }
  }
  return GST_FLOW_OK;

  /* ERRORS */
activate_failed : {
  GST_ELEMENT_ERROR(trans, RESOURCE, SETTINGS,
                    ("failed to activate bufferpool"),
                    ("failed to activate bufferpool"));
  return GST_FLOW_ERROR;
}

alloc_failed : {
  GST_DEBUG_OBJECT(trans, "could not allocate buffer from pool");
  return ret;
}

no_outcaps : {
  GST_DEBUG_OBJECT(trans, "no output caps, source pad has been deactivated");
  return GST_FLOW_FLUSHING;
}

config_failed : {
  if (pool) {
    gst_object_unref(pool);
  }
  return GST_FLOW_ERROR;
}
}

/* transform */
static GstFlowReturn gst_raw2depth_transform(GstBaseTransform *trans,
                                             GstBuffer *inbuf,
                                             GstBuffer *outbuf) {
  GstRaw2depth *raw2depth = GST_RAW2DEPTH(trans);
  GstFlowReturn ret;

  GST_DEBUG_OBJECT(raw2depth, "transform");

  ret = raw2depth->convert(trans, inbuf, outbuf);

  return ret;
}