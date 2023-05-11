gst_test_sources = [
  'common/tofmeta.cc',
  'common/tofformat.cc',
  'tofparser/tofparser.cc',
  'raw2depth/raw2depth.cc',
  'gtest-main.cc'
]

gst_check_dep = dependency('gstreamer-check-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_check_deps'])

exe = executable('gtest-main', gst_test_sources,
  include_directories : [incdir],
  link_with : [plugin_lib],
  dependencies : [gst_dep, gstbase_dep, gst_check_dep, math_dep, gtest_dep, gmock_dep, spdlog_dep]
)

env = environment()
tofplugin_path = join_paths(meson.project_build_root(), 'lib/gst-tof/')
env.append('GST_PLUGIN_PATH', tofplugin_path)
# env.set('GST_DEBUG', '2,baseparse:6,tofparser:6')

test('plugin_tests', exe, env:env, timeout : 10, workdir : test_dir)