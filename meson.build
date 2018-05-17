project('rayni-staging',
        'cpp',
        license : 'GPLv3+',
        version : '0.0.1',
        meson_version : '>=0.46.0',
        default_options : [
            'b_ndebug=if-release',
            'buildtype=release',
            'cpp_std=c++17',
            'warning_level=3',
            'werror=true'
        ])

cpp = meson.get_compiler('cpp')
cpp_args = []
cpp_link_args = []

if get_option('extra_warnings')
    extra_warnings_cpp_args = [
        '-Wdouble-promotion',
        '-Wformat=2',
        '-Wold-style-cast',
        '-Woverloaded-virtual',
        '-Wshadow',
        '-Wsign-conversion',
        '-Wsuggest-override',
        '-Wzero-as-null-pointer-constant'
    ]
    cpp_args += cpp.get_supported_arguments(extra_warnings_cpp_args)
endif

if get_option('libcxx')
    cpp_args += ['-stdlib=libc++']
    cpp_link_args += ['-stdlib=libc++', '-lc++abi']

    # libc++ does not fully implement C++11 thread_local on its own. Declaring a
    # variable with a destructor thread_local and linking outputs:
    # undefined reference to symbol '__cxa_thread_atexit@@CXXABI_1.3.7'
    # Link with libsupc++ from GCC to fix this.
    # TODO: Has this been fixed in libc++ yet?
    cpp_link_args += ['-lsupc++']

    # TODO: For std::experimental::filesystem. Remove when libc++ with C++17 is released.
    cpp_link_args += ['-llibc++experimental']
else
    # TODO: For std::filesystem in libstdc++. Still unstable? Or why is it not in libstdc++ proper yet?
    cpp_link_args += ['-lstdc++fs']
endif

if get_option('ofast_native') and get_option('buildtype') == 'release'
    cpp_args += ['-Ofast', '-march=native', '-funroll-loops']
    cpp_link_args += ['-Ofast', '-march=native']
endif

add_global_arguments(cpp_args, language : 'cpp')
add_global_link_arguments(cpp_link_args, language : 'cpp')

subdir('external')
subdir('src')
