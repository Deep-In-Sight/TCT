/* GStreamer
 * Copyright (C) 2023 DeepInSight <lnlinh93@dinsight.ai>
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
 * SECTION:element-gstfilesource
 *
 * The filesource element does read a file and output buffers according
 * to the negotiated caps.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v filesource, location=<file_path> ! fakesink
 * ]|
 * read a file and output default format buffers. fakesink then consume it
 * without doing anything.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>
#include "filesource/filesource.h"

GST_DEBUG_CATEGORY_STATIC (gst_filesource_debug_category);
#define GST_CAT_DEFAULT gst_filesource_debug_category

/* prototypes */


static void gst_filesource_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_filesource_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_filesource_dispose (GObject * object);
static void gst_filesource_finalize (GObject * object);

static GstCaps *gst_filesource_get_caps (GstBaseSrc * src, GstCaps * filter);
static gboolean gst_filesource_negotiate (GstBaseSrc * src);
static GstCaps *gst_filesource_fixate (GstBaseSrc * src, GstCaps * caps);
static gboolean gst_filesource_set_caps (GstBaseSrc * src, GstCaps * caps);
static gboolean gst_filesource_decide_allocation (GstBaseSrc * src,
    GstQuery * query);
static gboolean gst_filesource_start (GstBaseSrc * src);
static gboolean gst_filesource_stop (GstBaseSrc * src);
static void gst_filesource_get_times (GstBaseSrc * src, GstBuffer * buffer,
    GstClockTime * start, GstClockTime * end);
static gboolean gst_filesource_get_size (GstBaseSrc * src, guint64 * size);
static gboolean gst_filesource_is_seekable (GstBaseSrc * src);
static gboolean gst_filesource_prepare_seek_segment (GstBaseSrc * src,
    GstEvent * seek, GstSegment * segment);
static gboolean gst_filesource_do_seek (GstBaseSrc * src, GstSegment * segment);
static gboolean gst_filesource_unlock (GstBaseSrc * src);
static gboolean gst_filesource_unlock_stop (GstBaseSrc * src);
static gboolean gst_filesource_query (GstBaseSrc * src, GstQuery * query);
static gboolean gst_filesource_event (GstBaseSrc * src, GstEvent * event);
static GstFlowReturn gst_filesource_create (GstBaseSrc * src, guint64 offset,
    guint size, GstBuffer ** buf);
static GstFlowReturn gst_filesource_alloc (GstBaseSrc * src, guint64 offset,
    guint size, GstBuffer ** buf);
static GstFlowReturn gst_filesource_fill (GstBaseSrc * src, guint64 offset,
    guint size, GstBuffer * buf);

enum
{
  PROP_0
};

/* pad templates */

static GstStaticPadTemplate gst_filesource_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstFilesource, gst_filesource, GST_TYPE_BASE_SRC,
  GST_DEBUG_CATEGORY_INIT (gst_filesource_debug_category, "filesource", 0,
  "debug category for filesource element"));

static void
gst_filesource_class_init (GstFilesourceClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseSrcClass *base_src_class = GST_BASE_SRC_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_filesource_src_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_filesource_set_property;
  gobject_class->get_property = gst_filesource_get_property;
  gobject_class->dispose = gst_filesource_dispose;
  gobject_class->finalize = gst_filesource_finalize;
  base_src_class->get_caps = GST_DEBUG_FUNCPTR (gst_filesource_get_caps);
  base_src_class->negotiate = GST_DEBUG_FUNCPTR (gst_filesource_negotiate);
  base_src_class->fixate = GST_DEBUG_FUNCPTR (gst_filesource_fixate);
  base_src_class->set_caps = GST_DEBUG_FUNCPTR (gst_filesource_set_caps);
  base_src_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_filesource_decide_allocation);
  base_src_class->start = GST_DEBUG_FUNCPTR (gst_filesource_start);
  base_src_class->stop = GST_DEBUG_FUNCPTR (gst_filesource_stop);
  base_src_class->get_times = GST_DEBUG_FUNCPTR (gst_filesource_get_times);
  base_src_class->get_size = GST_DEBUG_FUNCPTR (gst_filesource_get_size);
  base_src_class->is_seekable = GST_DEBUG_FUNCPTR (gst_filesource_is_seekable);
  base_src_class->prepare_seek_segment = GST_DEBUG_FUNCPTR (gst_filesource_prepare_seek_segment);
  base_src_class->do_seek = GST_DEBUG_FUNCPTR (gst_filesource_do_seek);
  base_src_class->unlock = GST_DEBUG_FUNCPTR (gst_filesource_unlock);
  base_src_class->unlock_stop = GST_DEBUG_FUNCPTR (gst_filesource_unlock_stop);
  base_src_class->query = GST_DEBUG_FUNCPTR (gst_filesource_query);
  base_src_class->event = GST_DEBUG_FUNCPTR (gst_filesource_event);
  base_src_class->create = GST_DEBUG_FUNCPTR (gst_filesource_create);
  base_src_class->alloc = GST_DEBUG_FUNCPTR (gst_filesource_alloc);
  base_src_class->fill = GST_DEBUG_FUNCPTR (gst_filesource_fill);

}

static void
gst_filesource_init (GstFilesource *filesource)
{
}

void
gst_filesource_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstFilesource *filesource = GST_FILESOURCE (object);

  GST_DEBUG_OBJECT (filesource, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_filesource_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstFilesource *filesource = GST_FILESOURCE (object);

  GST_DEBUG_OBJECT (filesource, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_filesource_dispose (GObject * object)
{
  GstFilesource *filesource = GST_FILESOURCE (object);

  GST_DEBUG_OBJECT (filesource, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_filesource_parent_class)->dispose (object);
}

void
gst_filesource_finalize (GObject * object)
{
  GstFilesource *filesource = GST_FILESOURCE (object);

  GST_DEBUG_OBJECT (filesource, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_filesource_parent_class)->finalize (object);
}

/* get caps from subclass */
static GstCaps *
gst_filesource_get_caps (GstBaseSrc * src, GstCaps * filter)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "get_caps");

  return NULL;
}

/* decide on caps */
static gboolean
gst_filesource_negotiate (GstBaseSrc * src)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "negotiate");

  return TRUE;
}

/* called if, in negotiation, caps need fixating */
static GstCaps *
gst_filesource_fixate (GstBaseSrc * src, GstCaps * caps)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "fixate");

  return NULL;
}

/* notify the subclass of new caps */
static gboolean
gst_filesource_set_caps (GstBaseSrc * src, GstCaps * caps)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "set_caps");

  return TRUE;
}

/* setup allocation query */
static gboolean
gst_filesource_decide_allocation (GstBaseSrc * src, GstQuery * query)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "decide_allocation");

  return TRUE;
}

/* start and stop processing, ideal for opening/closing the resource */
static gboolean
gst_filesource_start (GstBaseSrc * src)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "start");

  return TRUE;
}

static gboolean
gst_filesource_stop (GstBaseSrc * src)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "stop");

  return TRUE;
}

/* given a buffer, return start and stop time when it should be pushed
 * out. The base class will sync on the clock using these times. */
static void
gst_filesource_get_times (GstBaseSrc * src, GstBuffer * buffer,
    GstClockTime * start, GstClockTime * end)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "get_times");

}

/* get the total size of the resource in bytes */
static gboolean
gst_filesource_get_size (GstBaseSrc * src, guint64 * size)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "get_size");

  return TRUE;
}

/* check if the resource is seekable */
static gboolean
gst_filesource_is_seekable (GstBaseSrc * src)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "is_seekable");

  return TRUE;
}

/* Prepare the segment on which to perform do_seek(), converting to the
 * current basesrc format. */
static gboolean
gst_filesource_prepare_seek_segment (GstBaseSrc * src, GstEvent * seek,
    GstSegment * segment)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "prepare_seek_segment");

  return TRUE;
}

/* notify subclasses of a seek */
static gboolean
gst_filesource_do_seek (GstBaseSrc * src, GstSegment * segment)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "do_seek");

  return TRUE;
}

/* unlock any pending access to the resource. subclasses should unlock
 * any function ASAP. */
static gboolean
gst_filesource_unlock (GstBaseSrc * src)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "unlock");

  return TRUE;
}

/* Clear any pending unlock request, as we succeeded in unlocking */
static gboolean
gst_filesource_unlock_stop (GstBaseSrc * src)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "unlock_stop");

  return TRUE;
}

/* notify subclasses of a query */
static gboolean
gst_filesource_query (GstBaseSrc * src, GstQuery * query)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "query");

  return TRUE;
}

/* notify subclasses of an event */
static gboolean
gst_filesource_event (GstBaseSrc * src, GstEvent * event)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "event");

  return TRUE;
}

/* ask the subclass to create a buffer with offset and size, the default
 * implementation will call alloc and fill. */
static GstFlowReturn
gst_filesource_create (GstBaseSrc * src, guint64 offset, guint size,
    GstBuffer ** buf)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "create");

  return GST_FLOW_OK;
}

/* ask the subclass to allocate an output buffer. The default implementation
 * will use the negotiated allocator. */
static GstFlowReturn
gst_filesource_alloc (GstBaseSrc * src, guint64 offset, guint size,
    GstBuffer ** buf)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "alloc");

  return GST_FLOW_OK;
}

/* ask the subclass to fill the buffer with data from offset and size */
static GstFlowReturn
gst_filesource_fill (GstBaseSrc * src, guint64 offset, guint size, GstBuffer * buf)
{
  GstFilesource *filesource = GST_FILESOURCE (src);

  GST_DEBUG_OBJECT (filesource, "fill");

  return GST_FLOW_OK;
}
