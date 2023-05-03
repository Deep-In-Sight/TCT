project('TCT', 'c', 'cpp', version : '0.1', license : 'LGPL')

# cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_base_dep'])

math_dep = cxx.find_library('m', required : true)
thread_dep = cxx.find_library('pthread', required: true)

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')
gmock_dep = gtest_proj.get_variable('gmock_dep')
gtest_main_dep = gtest_proj.get_variable('gtest_main_dep')

subdir('gst-tofplugin')