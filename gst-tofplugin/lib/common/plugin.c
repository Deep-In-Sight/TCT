#include <gst/gst.h>
#include <lib/common/plugin.h>
#include <lib/tofparser/tofparser.h>

static gboolean plugin_init(GstPlugin* plugin) {
  gboolean ret = FALSE;

  ret |= gst_element_register(plugin, "tofparser", GST_RANK_NONE,
                              GST_TYPE_TOFPARSER);

  return ret;
}

#define GST_API_VERSION "1.0"
#define GST_LICENSE "LGPL"
#define GST_PACKAGE_NAME "GStreamer ToF Plugin"
#define GST_PACKAGE_ORIGIN "dinsight.ai"
// rename this to the folder name that contain this plugin
#define PACKAGE "gst-tofplugin"
#define PACKAGE_VERSION "0.1"

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, tofplugin,
                  "Gstreamer Plugin for TCT", plugin_init, PACKAGE_VERSION,
                  GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
