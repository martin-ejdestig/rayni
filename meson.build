project('rayni-staging', 'cpp',
	license : 'GPLv3+',
	default_options : [
		'cpp_std=c++14',
		'warning_level=3',
		'werror=true'
	])

if get_option('double_precision')
	add_global_arguments('-DRAYNI_DOUBLE_PRECISION', language : 'cpp')
endif

if get_option('extra_warnings')
	extra_warnings = [
		'-Wold-style-cast',
		'-Woverloaded-virtual',
		'-Wformat=2',
		'-Wsign-conversion'
	]
	if meson.get_compiler('cpp').get_id() == 'gcc'
		extra_warnings += [
			'-Wdouble-promotion',
			'-Wsuggest-override',
			'-Wzero-as-null-pointer-constant'
		]
	endif
	add_global_arguments(extra_warnings, language : 'cpp')
endif

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
		'src/lib/math/enum.h',
		'src/lib/math/hash.h',
		'src/lib/math/math.h',
		'src/lib/string/string.cpp',
		'src/lib/string/string.h',
		'src/lib/system/command.cpp',
		'src/lib/system/command.h',
		'src/lib/time/duration_formatter.cpp',
		'src/lib/time/duration_formatter.h',
		'src/lib/time/scoped_stopwatch.cpp',
		'src/lib/time/scoped_stopwatch.h',
		'src/lib/time/stopwatch.cpp',
		'src/lib/time/stopwatch.h'
	])

gtest = dependency('gtest', main : true)

lib_unit_tests = executable('librayni-unit_tests',
	include_directories : src_incdir,
	link_with : lib,
	dependencies : gtest,
	sources : [
		'src/unit_tests/lib/color.cpp',
		'src/unit_tests/lib/containers/fixed_size_stack.cpp',
		'src/unit_tests/lib/containers/listener_list.cpp',
		'src/unit_tests/lib/containers/variant.cpp',
		'src/unit_tests/lib/expect_real_eq.h',
		'src/unit_tests/lib/image.cpp',
		'src/unit_tests/lib/math/enum.cpp',
		'src/unit_tests/lib/math/hash.cpp',
		'src/unit_tests/lib/math/math.cpp',
		'src/unit_tests/lib/string/string.cpp',
		'src/unit_tests/lib/system/command.cpp',
		'src/unit_tests/lib/time/duration_formatter.cpp',
		'src/unit_tests/lib/time/scoped_stopwatch.cpp',
		'src/unit_tests/lib/time/stopwatch.cpp'
	])

test('librayni tests', lib_unit_tests)

