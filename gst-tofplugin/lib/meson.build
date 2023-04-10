# TOF Plugin
plugin_sources = [
  'common/tofmeta.c',
  'common/tofformat.c',
  'tofparser/tofparser.c',
  'raw2depth/raw2depth.c',
  'common/plugin.c'
]

tof_plugin = library('tofplugin',
  plugin_sources,
  include_directories : [incdir],
  dependencies : [gst_dep, gstbase_dep],
  install : false
)
