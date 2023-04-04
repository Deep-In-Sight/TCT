tests = [
  'common/tofmeta.c',
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

    # env = environment()
    # gst_plugin_path = env.get('GST_PLUGIN_PATH')
    # env.set('GST_PLUGIN_PATH', gst_plugin_path + ':' + meson.project_build_root() + '/')

  test(test_name, exe, timeout : 3 * 60)

endforeach