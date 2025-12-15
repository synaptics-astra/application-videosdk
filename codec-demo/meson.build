project('codec-demo', ['c','cpp'],
  version: '1.0',
  default_options: ['warning_level=2']
)

egl_dep = dependency('egl', required: true)
gles2_dep = dependency('glesv2', required: true)
drm_dep = dependency('libdrm', required: true)
gbm_dep = dependency('gbm', required: true)
glib_dep = dependency('glib-2.0', required: true)
gst_dep = dependency('gstreamer-1.0', required: true)
gst_base_dep = dependency('gstreamer-base-1.0', required: true)
avcodec_dep = dependency('libavcodec', required: true)
avutil_dep  = dependency('libavutil', required: true)
gst_app_dep  = dependency('gstreamer-app-1.0', required: true)

# List of source files
codec_demo_sources = [
  'main.c',
  'tsemaphore.c',
  'queue.c',
  'config.c',
  'instance.c',
  'gsttxpipeline.c',
  'filereader.c',
  'gstrxpipeline.c',
  'UIInterface.c',
  'v4l2camera.c',
  'encoder_interface_sw.c',
  'swh264enc.c',
  'decoder_interface_sw.c',
  'swh264dec.c',
  'gbm-mem-utils.c',
  'neon_utils.c'
]
codec_demo_includes = include_directories('.')

curr_mach   = get_option('curr_mach')
v4l2_enable = get_option('v4l2_enable')

add_project_arguments(['-DCURR_MACH="' + curr_mach + '"'], language: 'c')

if curr_mach == 'dolphin'
  add_project_arguments('-DCURR_MACH_DOLPHIN', language : 'c')
elif curr_mach == 'platypus'
  add_project_arguments('-DCURR_MACH_PLATYPUS', language : 'c')
elif curr_mach == 'myna2'
  add_project_arguments('-DCURR_MACH_MYNA2', language : 'c')
elif curr_mach == 'klamath'
  add_project_arguments('-DCURR_MACH_KLAMATH', language : 'c')
else
  warning('Unknown curr_mach value: ' + curr_mach)
endif

if v4l2_enable
  add_project_arguments('-DV4L2_SUPPORT', language : 'c')
endif

if v4l2_enable
  codec_demo_sources += [
    'hwh264enc.c',
    'encoder_interface_hwv4l2.c',
    'hwh264dec.c',
    'decoder_interface_hwv4l2.c',
    'v4l2hrx.c'
  ]
endif

executable('codec-demo',
  codec_demo_sources,
  include_directories: codec_demo_includes,
  cpp_args: ['-g'],
  link_args: ['-lm'],
  dependencies: [egl_dep, gles2_dep, drm_dep, gbm_dep, glib_dep, gst_dep, avcodec_dep, avutil_dep, gst_app_dep],
  install: true
)
