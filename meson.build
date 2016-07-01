project('rayni-staging',
        'cpp',
        license : 'GPLv3+',
        version : '0.0.1',
        default_options : [
            'cpp_std=c++14',
            'warning_level=3',
            'werror=true'
        ])

cpp = meson.get_compiler('cpp')
cpp_args = []
cpp_link_args = []

if get_option('extra_warnings')
    cpp_args += [
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

if cpp.get_id() == 'gcc'
    # TODO: For std::experimental::filesystem. Remove when libstdc++ with C++17 is released.
    cpp_link_args += ['-lstdc++fs']
endif

if cpp.get_id() == 'clang'
    cpp_args += ['-stdlib=libc++']
    cpp_link_args += ['-stdlib=libc++', '-lc++abi']

    # libc++ does not fully implement C++11 thread_local on its own. Declaring a
    # variable with a destructor thread_local and linking outputs:
    # undefined reference to symbol '__cxa_thread_atexit@@CXXABI_1.3.7'
    # Link with libsupc++ from GCC to fix this.
    # TODO: Has this been fixed in libc++ yet?
    cpp_link_args += ['-lsupc++']

    # TODO: https://llvm.org/bugs/show_bug.cgi?id=21629
    cpp_args += ['-Wno-missing-braces']

    # TODO: For std::experimental::filesystem. Remove when libc++ with C++17 is released.
    cpp_link_args += ['-llibc++experimental']
endif

if get_option('link_time_optimization')
    cpp_args += ['-flto']
    cpp_link_args += ['-flto']
endif

if get_option('ofast_native')
    cpp_args += ['-DNDEBUG', '-Ofast', '-march=native', '-funroll-loops']
    cpp_link_args += ['-Ofast', '-march=native']
endif

if get_option('sanitize_address')
    cpp_args += ['-fsanitize=address', '-fno-omit-frame-pointer']
    cpp_link_args += ['-fsanitize=address', '-fno-omit-frame-pointer']
endif

if get_option('sanitize_control_flow_integrity')
    cpp_args += ['-fsanitize=cfi']
    cpp_link_args += ['-fsanitize=cfi']
endif

if get_option('sanitize_memory')
    cpp_args += ['-fsanitize=memory', '-fsanitize-memory-track-origins', '-fno-omit-frame-pointer']
    cpp_link_args += ['-fsanitize=memory', '-fsanitize-memory-track-origins', '-fno-omit-frame-pointer']
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
add_global_link_arguments(cpp_link_args, language : 'cpp')

subdir('external')
subdir('src')
