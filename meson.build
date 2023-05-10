project('TCT', 'c', 'cpp', version : '0.1', license : 'LGPL')

# cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

# common libs
math_dep = cxx.find_library('m', required : true)
thread_dep = cxx.find_library('pthread', required: true)

# gstreamer
gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_base_dep'])

# gtest
gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')
gmock_dep = gtest_proj.get_variable('gmock_dep')
gtest_main_dep = gtest_proj.get_variable('gtest_main_dep')

# spdlog. 
# vscode meson extension does not support individual test target well so, to be
# able to run gsttofplugin test, we have to disable everything else then select
# run all tests
spdlog_proj = subproject('spdlog', default_options : 'tests=false')
spdlog_dep = spdlog_proj.get_variable('spdlog_dep')

incdir = include_directories('lib')
subdir('lib')
subdir('tests')