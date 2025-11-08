# Based on
project('gba-rumble', 'cpp',
  version: '1.0.1',
  license: 'MIT',
  meson_version: '>=0.54.0',
  default_options: ['warning_level=0', 'cpp_std=none'])

gba_rumble_dep = declare_dependency(
  include_directories: ['.'])

meson.override_dependency('gba-rumble', gba_rumble_dep)
