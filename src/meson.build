src_incdir = include_directories('.')

config_data = configuration_data()
config_data.set10('double_precision', get_option('double_precision'))
config_data.set('ray_triangle_intersection', get_option('ray_triangle_intersection').to_upper())

config_header = configure_file(input : 'config.h.in',
                               output : 'config.h',
                               configuration : config_data)

subdir('lib')
subdir('unit_tests')
