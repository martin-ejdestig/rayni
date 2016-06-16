src_incdir = include_directories('.')

config_data = configuration_data()
config_data.set('RAYNI_DOUBLE_PRECISION', get_option('double_precision'))

config_header = configure_file(input : 'config.h.in',
                               output : 'config.h',
                               configuration : config_data)

git_describe_version_header = vcs_tag(input : 'git_describe_version.h.in',
                                      output : 'git_describe_version.h',
                                      fallback : meson.project_version(),
                                      command : ['git', 'describe', '--always', '--first-parent', '--dirty'])

subdir('lib')
subdir('unit_tests')
