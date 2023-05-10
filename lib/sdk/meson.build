# SDK
sdk_sources = [
  'inspector/inspector.cc',
  'inspector/inspector-client.cc',
  'inspector/inspector-scanner.cc',
  'inspector/inspector-tracker.cc',
  'inspector/inspector-histogram.cc',
  'inspector/inspector-bitmap.cc',
]

sdk_lib = library('sdk',
  sdk_sources,
  include_directories : [incdir],
  dependencies : [gst_dep, gstbase_dep, math_dep, thread_dep],
  install : false
)
