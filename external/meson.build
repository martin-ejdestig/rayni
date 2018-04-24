gtest_dir = 'googletest/googletest'
gtest_incdir = include_directories(join_paths(gtest_dir, 'include'), is_system : true)

gtest_lib = static_library('gtest',
                           cpp_args : ['-w'],
                           include_directories : [include_directories(gtest_dir), gtest_incdir],
                           sources : [
                               join_paths(gtest_dir, 'src', 'gtest-all.cc'),
                               join_paths(gtest_dir, 'src', 'gtest_main.cc')
                           ])

gtest_compile_args = []

if get_option('extra_warnings')
    # Clang warns for -Wzero-as-null-pointer-constant in googletest headers even
    # though -isystem is used. See https://bugs.llvm.org/show_bug.cgi?id=33771#c5
    # See https://github.com/google/googletest/issues/1323 for googletest issue.
    if meson.get_compiler('cpp').get_id() == 'clang'
        gtest_compile_args += ['-Wno-zero-as-null-pointer-constant']
    endif
endif

gtest_dependency = declare_dependency(dependencies : dependency('threads'),
                                      compile_args : gtest_compile_args,
                                      include_directories : gtest_incdir,
                                      link_with : gtest_lib)

sork = find_program('sork/sork.py')
run_target('analyze', command : [sork, '--build-path', meson.build_root(), 'analyze', meson.source_root()])
run_target('style_check', command : [sork, '--build-path', meson.build_root(), 'check', meson.source_root()])
