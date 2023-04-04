# TOF Plugin
plugin_sources = [
   'filter1/filter1.c',
   'tofparser/tofparser.c',
   'common/tofmeta.c',
   'common/plugin.c'
]

tof_plugin = library('tofplugin',
  plugin_sources,
  include_directories : [incdir],
  dependencies : [gst_dep, gstbase_dep],
  install : false
)
