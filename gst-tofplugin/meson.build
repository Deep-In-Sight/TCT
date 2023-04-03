# TOF Plugin
plugin_sources = [
   'lib/filter1/filter1.c',
   'lib/tofparser/tofparser.c',
   'lib/plugin.c'
]

plugin = library('tofplugin',
  plugin_sources,
  dependencies : [gst_dep, gstbase_dep],
  install : false
)
