sdk_test_sources = [
  'inspector/inspector.cc',
  'inspector/inspector-client.cc',
  'inspector/inspector-scanner.cc',
  'inspector/inspector-tracker.cc',
  'inspector/inspector-histogram.cc',
  'gtest-main.cc'
]

gst_check_dep = dependency('gstreamer-check-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_check_deps'])

exe = executable('gtest-main', sdk_test_sources,
  include_directories : [incdir],
  link_with : [sdk_lib],
  dependencies : [gst_dep, gstbase_dep, gst_check_dep, math_dep, gtest_dep, gmock_dep, spdlog_dep]
)

env = environment()
tofplugin_path = join_paths(meson.project_build_root(), 'lib/gst-tof')
env.append('GST_PLUGIN_PATH', tofplugin_path)
# env.set('GST_DEBUG', '2,baseparse:6,tofparser:6')

test('sdk_tests', exe, env:env, timeout : 10, workdir : test_dir)