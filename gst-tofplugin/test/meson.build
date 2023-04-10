tests = [
  'common/tofmeta.c',
  'common/tofformat.c',
  'tofparser/tofparser.c',
  'raw2depth/raw2depth.c'
]

gst_check_dep = dependency('gstreamer-check-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_check_deps'])

foreach t : tests
  fname = t
  test_name = fname.split('.')[0].split('/')[1]


  exe = executable(test_name, fname,
    include_directories : [incdir],
    link_with : [tof_plugin],
    dependencies : [gst_dep, gstbase_dep, gst_check_dep]
  )

  env = environment()
  tofplugin_path = join_paths(meson.project_build_root(), 'gst-tofplugin/lib')
  env.append('GST_PLUGIN_PATH', tofplugin_path)
  # env.set('GST_DEBUG', '2,baseparse:6,tofparser:6')

  test(test_name, exe, env:env, timeout : 10)

endforeach