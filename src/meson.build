src_incdir = include_directories('.')

config_data = configuration_data()
config_data.set('RAYNI_DOUBLE_PRECISION', get_option('double_precision'))
config_data.set('RAYNI_RAY_TRIANGLE_INTERSECTION',
                'RAYNI_RAY_TRIANGLE_INTERSECTION_' + get_option('ray_triangle_intersection').to_upper())

config_header = configure_file(input : 'config.h.in',
                               output : 'config.h',
                               configuration : config_data)

subdir('lib')
subdir('unit_tests')
