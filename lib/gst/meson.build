# TOF Plugin
plugin_sources = [
  'common/tofmeta.cc',
  'common/tofformat.cc',
  'tofparser/tofparser.cc',
  'raw2depth/raw2depth.cc',
  'common/plugin.cc'
]

plugin_lib = library('tofplugin',
  plugin_sources,
  include_directories : [incdir],
  dependencies : [gst_dep, gstbase_dep, math_dep, thread_dep],
  install : false
)
