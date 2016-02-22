project('rayni-staging', 'cpp',
	license : 'GPLv3+',
	default_options : [
		'cpp_std=c++14',
		'warning_level=3',
		'werror=true'
	])

### Dependencies ###

gtest = dependency('gtest', main : true)

### Compiler and linker flags ###

cpp = meson.get_compiler('cpp')
cpp_args = []
cpp_link_args = []

if get_option('extra_warnings')
	cpp_args = [
		'-Wold-style-cast',
		'-Woverloaded-virtual',
		'-Wformat=2',
		'-Wsign-conversion'
	]
	if cpp.get_id() == 'gcc'
		cpp_args += [
			'-Wdouble-promotion',
			'-Wsuggest-override',
			'-Wzero-as-null-pointer-constant'
		]
	endif
endif

if get_option('ofast_native')
	cpp_args += ['-DNDEBUG', '-Ofast', '-march=native', '-funroll-loops']
	cpp_link_args = ['-Ofast', '-march=native']
endif

if get_option('sanitize_address')
	cpp_args += ['-fsanitize=address']
	cpp_link_args += ['-fsanitize=address']
endif

if get_option('sanitize_thread')
	cpp_args += ['-fsanitize=thread', '-fPIE']
	cpp_link_args += ['-fsanitize=thread', '-pie']
endif

if get_option('sanitize_undefined')
	cpp_args += ['-fsanitize=undefined']
	cpp_link_args += ['-fsanitize=undefined']
endif

add_global_arguments(cpp_args, language : 'cpp')

### librayni ###

config_data = configuration_data()
config_data.set('RAYNI_DOUBLE_PRECISION', get_option('double_precision'))

config_header = configure_file(
	input : 'src/config.h.in',
	output : 'config.h',
	configuration : config_data)

git_describe_version_header = vcs_tag(
	input : 'src/git_describe_version.h.in',
	output : 'git_describe_version.h',
	fallback : '<git describe fallback>',
	command : ['git', 'describe', '--always', '--first-parent', '--dirty'])

src_incdir = include_directories('src')

lib = static_library('rayni',
	include_directories : src_incdir,
	sources : [
		'src/lib/color.cpp',
		'src/lib/color.h',
		'src/lib/containers/fixed_size_stack.h',
		'src/lib/containers/listener_list.h',
		'src/lib/containers/variant.cpp',
		'src/lib/containers/variant.h',
		'src/lib/image.cpp',
		'src/lib/image.h',
		'src/lib/math/aabb.cpp',
		'src/lib/math/aabb.h',
		'src/lib/math/animated_transform.cpp',
		'src/lib/math/animated_transform.h',
		'src/lib/math/decomposed_matrix4x4.cpp',
		'src/lib/math/decomposed_matrix4x4.h',
		'src/lib/math/enum.h',
		'src/lib/math/hash.h',
		'src/lib/math/math.h',
		'src/lib/math/matrix4x4.cpp',
		'src/lib/math/matrix4x4.h',
		'src/lib/math/quaternion.h',
		'src/lib/math/ray.h',
		'src/lib/math/transform.cpp',
		'src/lib/math/transform.h',
		'src/lib/math/vector3.h',
		'src/lib/math/vector4.h',
		'src/lib/string/string.cpp',
		'src/lib/string/string.h',
		'src/lib/system/command.cpp',
		'src/lib/system/command.h',
		'src/lib/time/duration_formatter.cpp',
		'src/lib/time/duration_formatter.h',
		'src/lib/time/scoped_stopwatch.cpp',
		'src/lib/time/scoped_stopwatch.h',
		'src/lib/time/stopwatch.cpp',
		'src/lib/time/stopwatch.h',
		'src/lib/version_info.cpp',
		'src/lib/version_info.h',
		config_header,
		git_describe_version_header
	])

lib_unit_tests = executable('librayni-unit_tests',
	include_directories : src_incdir,
	link_args : cpp_link_args,
	link_with : lib,
	dependencies : gtest,
	sources : [
		'src/unit_tests/lib/color.cpp',
		'src/unit_tests/lib/containers/fixed_size_stack.cpp',
		'src/unit_tests/lib/containers/listener_list.cpp',
		'src/unit_tests/lib/containers/variant.cpp',
		'src/unit_tests/lib/image.cpp',
		'src/unit_tests/lib/math/aabb.cpp',
		'src/unit_tests/lib/math/animated_transform.cpp',
		'src/unit_tests/lib/math/decomposed_matrix4x4.cpp',
		'src/unit_tests/lib/math/enum.cpp',
		'src/unit_tests/lib/math/hash.cpp',
		'src/unit_tests/lib/math/math.cpp',
		'src/unit_tests/lib/math/matrix4x4.cpp',
		'src/unit_tests/lib/math/quaternion.cpp',
		'src/unit_tests/lib/math/transform.cpp',
		'src/unit_tests/lib/math/vector3.cpp',
		'src/unit_tests/lib/math/vector4.cpp',
		'src/unit_tests/lib/string/string.cpp',
		'src/unit_tests/lib/system/command.cpp',
		'src/unit_tests/lib/time/duration_formatter.cpp',
		'src/unit_tests/lib/time/scoped_stopwatch.cpp',
		'src/unit_tests/lib/time/stopwatch.cpp',
		'src/unit_tests/lib/version_info.cpp'
	])

test('librayni tests', lib_unit_tests)
