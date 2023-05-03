# TOF Plugin
plugin_sources = [
  'common/tofmeta.cpp',
  'common/tofformat.cpp',
  'tofparser/tofparser.cpp',
  'raw2depth/raw2depth.cpp',
  'inspector/inspector.cpp',
  'common/plugin.cpp'
]

tof_plugin = library('tofplugin',
  plugin_sources,
  include_directories : [incdir],
  dependencies : [gst_dep, gstbase_dep, math_dep, thread_dep],
  install : false
)
