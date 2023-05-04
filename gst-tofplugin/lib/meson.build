# TOF Plugin
plugin_sources = [
  'common/tofmeta.cc',
  'common/tofformat.cc',
  'tofparser/tofparser.cc',
  'raw2depth/raw2depth.cc',
  'inspector/inspector.cc',
  'inspector/inspector-client.cc',
  # 'inspector/inspector-scanner.cc',
  'common/plugin.cc'
]

tof_plugin = library('tofplugin',
  plugin_sources,
  include_directories : [incdir],
  dependencies : [gst_dep, gstbase_dep, math_dep, thread_dep],
  install : false
)
