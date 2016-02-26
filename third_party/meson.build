# TODO: Want to use Meson's gtest dependency but does not always work since it
#       prefers prebuilt libs. (E.g. linking with libc++ fails if libs are built
#       with libstdc++.)
# TODO: Add git submodule or use Meson's wrap tool? Leaning towards submodule
#       pointing to https://github.com/google/googletest.git instead.

gtest_dir = '/usr/src/gtest' # Where Arch, Debian and Fedora put the gtest source files.

gtest_incdir = include_directories(gtest_dir, is_system : true)

libgtest = static_library('gtest',
	cpp_args : ['-w'],
	include_directories : gtest_incdir,
	sources : [
		gtest_dir + '/src/gtest-all.cc',
		gtest_dir + '/src/gtest_main.cc',
	])

gtest = declare_dependency(
	dependencies : dependency('threads'),
	include_directories : gtest_incdir,
	link_with : libgtest)
