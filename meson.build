project('TCT', 'c', 'cpp', version : '0.1', license : 'LGPL')

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_base_dep'])

math_dep = cc.find_library('m', required : true)

subdir('gst-tofplugin')