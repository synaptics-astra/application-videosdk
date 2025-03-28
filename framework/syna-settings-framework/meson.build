project('syna-settings-framework', ['cpp'],
  version: '1.0',
  default_options: ['warning_level=2']
)

# List of source files
synasettingsframework_sources = files('syna-settings-framework.cpp')
synasettingsframework_includes = include_directories('.')

synasettingsframework_lib = shared_library('syna-settings-framework',
  synasettingsframework_sources,
  include_directories: synasettingsframework_includes,
  link_args: ['-lm'],
  install: true
)

test_sources = files('syna-settings-framework-test.cpp')
test_includes = include_directories('.')

unit_test_executable = executable('syna-settings-framework_test',
  test_sources,
  include_directories: [synasettingsframework_includes, test_includes],
  link_args: ['-lm'],
  link_with: synasettingsframework_lib
)
