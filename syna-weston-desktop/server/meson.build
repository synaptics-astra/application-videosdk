cc = meson.get_compiler('c')

libweston_dep = dependency('libweston-10')
libweston_desk_dep = dependency('libweston-desktop-10')
wl_client_dep = dependency('wayland-client')
wl_server_dep = dependency('wayland-server')
weston_dep = dependency('weston')
dep_pixman = dependency('pixman-1', version: '>= 0.25.2')
dep_libm = cc.find_library('m')

weston_path = weston_dep.get_pkgconfig_variable('prefix')

srcs_syna_desktop = [
  'shell.c',
  'exposay.c',
  'input-panel.c',
  '../shared/shell-utils.c',
  weston_desktop_shell_server_protocol_h,
  weston_desktop_shell_protocol_c,
  input_method_unstable_v1_server_protocol_h,
  input_method_unstable_v1_protocol_c,
]

inc_dir_syna_desktop = include_directories('../')
message('Current build directory is: ' + meson.current_build_dir())

# Example of adjusting library path dynamically
lib_path = meson.current_build_dir() + '/../../recipe-sysroot/usr/lib/weston'
message('Library path set to: ' + lib_path)

link_args = ['-L' + lib_path, '-lexec_weston']

# Define your shared library or executable
plugin_weston_syna_desktop = shared_library(
  'syna-desktop-shell',
  srcs_syna_desktop,
  include_directories: inc_dir_syna_desktop,
  dependencies: [libweston_dep, libweston_desk_dep, weston_dep, wl_client_dep,
    wl_server_dep, dep_pixman, dep_libm],
  link_args: link_args,
  name_prefix: '',
  install: true
)
