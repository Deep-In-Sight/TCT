tests = [
  'common/tofmeta.cc',
  'common/tofformat.cc',
  'tofparser/tofparser.cc',
  'raw2depth/raw2depth.cc',
  'inspector/inspector.cc',
  'inspector/inspector-client.cc',
  'gtest-main.cc'
]

gst_check_dep = dependency('gstreamer-check-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_check_deps'])

exe = executable('gtest-main', tests,
  include_directories : [incdir],
  link_with : [tof_plugin],
  dependencies : [gst_dep, gstbase_dep, gst_check_dep, math_dep, gtest_dep, gmock_dep]
)

env = environment()
tofplugin_path = join_paths(meson.project_build_root(), 'gst-tofplugin/lib')
env.append('GST_PLUGIN_PATH', tofplugin_path)
# env.set('GST_DEBUG', '2,baseparse:6,tofparser:6')

test('gtest-main', exe, env:env, timeout : 10)