gtest_dir = 'googletest/googletest'
gtest_incdir = include_directories(join_paths(gtest_dir, 'include'), is_system : true)

gtest_lib = static_library('gtest',
                           cpp_args : ['-w'],
                           include_directories : [include_directories(gtest_dir), gtest_incdir],
                           sources : [
                               join_paths(gtest_dir, 'src', 'gtest-all.cc'),
                               join_paths(gtest_dir, 'src', 'gtest_main.cc')
                           ])

gtest_dep = declare_dependency(dependencies : dependency('threads'),
                               include_directories : gtest_incdir,
                               link_with : gtest_lib)
