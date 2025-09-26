cc = meson.get_compiler('c')

libweston_dep = dependency('libweston-13')
wl_client_dep = dependency('wayland-client')
wl_server_dep = dependency('wayland-server')
wl_cursor_dep = dependency('wayland-cursor')
weston_dep = dependency('weston')
dep_pixman = dependency('pixman-1', version: '>= 0.25.2')
dep_xkbcommon = dependency('xkbcommon', version: '>= 0.3.0')
dep_cairo = dependency('cairo')
dep_libpng = dependency('libpng')
dep_libm = cc.find_library('m')
dep_libutil = cc.find_library('util')

weston_path = weston_dep.get_pkgconfig_variable('prefix')
inc_dir_syna_desktop = include_directories('../')

exe_shell_desktop = executable(
  'syna-desktop-shell',
  'desktop-shell.c',
  'window.c',
  '../shared/cairo-util.c',
  '../shared/file-util.c',
  '../shared/frame.c',
  '../shared/os-compatibility.c',
  '../shared/image-loader.c',
  '../shared/process-util.c',
  weston_desktop_shell_client_protocol_h,
  weston_desktop_shell_protocol_c,
  xdg_shell_client_protocol_h,
  xdg_shell_protocol_c,
  text_cursor_position_client_protocol_h,
  text_cursor_position_protocol_c,
  relative_pointer_unstable_v1_client_protocol_h,
  relative_pointer_unstable_v1_protocol_c,
  pointer_constraints_unstable_v1_client_protocol_h,
  pointer_constraints_unstable_v1_protocol_c,
  viewporter_client_protocol_h,
  viewporter_protocol_c,
  include_directories: inc_dir_syna_desktop,
  dependencies: [ libweston_dep, weston_dep, dep_pixman ,
    dep_cairo, dep_libpng, wl_client_dep, dep_xkbcommon, wl_cursor_dep,
    dep_libm, dep_libutil ],
  install: true
)
