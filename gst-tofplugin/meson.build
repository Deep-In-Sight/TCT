# TOF Plugin
plugin_sources = [
  'src/filter1/filter1.c',
  'src/plugin.c'
  ]

plugin = library('tofplugin',
  plugin_sources,
  include_directories : ['include'],
  dependencies : [gst_dep, gstbase_dep],
  install : false
)