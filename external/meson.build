gtest_dir = 'googletest/googletest'
gtest_incdir = include_directories(gtest_dir + '/include', is_system : true)

libgtest = static_library('gtest',
	cpp_args : ['-w'],
	include_directories : [include_directories(gtest_dir), gtest_incdir],
	sources : [
		gtest_dir + '/src/gtest-all.cc',
		gtest_dir + '/src/gtest_main.cc',
	])

gtest = declare_dependency(
	dependencies : dependency('threads'),
	include_directories : gtest_incdir,
	link_with : libgtest)
