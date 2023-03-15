project('gst-template', 'c', version : '1.19.0.1', license : 'LGPL')

plugins_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

cc = meson.get_compiler('c')

gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_base_dep'])

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