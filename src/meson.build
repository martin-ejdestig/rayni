src_incdir = include_directories('.')

l1_cache_line_size = 0
if build_machine.system() == 'linux' and not meson.is_cross_build()
    cat_result = run_command('cat', '/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size')
    l1_cache_line_size = cat_result.returncode() == 0 ? cat_result.stdout().to_int() : 0
endif
if l1_cache_line_size == 0
    warning('Unable to determine L1 cache line size, using 64')
    l1_cache_line_size = 64
endif

config_data = configuration_data()
config_data.set('l1_cache_line_size', l1_cache_line_size)
config_data.set10('double_precision', get_option('double_precision'))
config_data.set('ray_triangle_intersection', get_option('ray_triangle_intersection').to_upper())

config_header = configure_file(input : 'config.h.in',
                               output : 'config.h',
                               configuration : config_data)

subdir('lib')
subdir('unit_tests')
