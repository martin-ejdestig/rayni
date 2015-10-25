# Download and build gtest from source.
#
# The reson for this is that some Linux distributions ship prebuilt libraries of
# Google Test while others do not. But more importantly, the authors of Google
# Test recommends against using prebuilt libraries. See:
#
# https://github.com/google/googletest/blob/master/googletest/docs/FAQ.md#why-is-it-not-recommended-to-install-a-pre-compiled-copy-of-google-test-for-example-into-usrlocal
#
# Use ExternalProject_Add to download the source but disable the build command
# and build with add_library() etc. so the compiler flags used for the rest of
# the project are used for gtest as well.

include(ExternalProject)

find_package(Threads REQUIRED)

ExternalProject_Add(gtest_download
                    URL https://github.com/google/googletest/archive/release-1.7.0.tar.gz
                    URL_MD5 4ff6353b2560df0afecfbda3b2763847
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
                    BUILD_COMMAND ""
                    CONFIGURE_COMMAND ""
                    INSTALL_COMMAND "")

ExternalProject_Get_Property(gtest_download source_dir)

set(GTEST_INCLUDE_DIR ${source_dir}/include)
set(GTEST_ALL_SRC ${source_dir}/src/gtest-all.cc)
set(GTEST_MAIN_SRC ${source_dir}/src/gtest_main.cc)

set_source_files_properties(${GTEST_ALL_SRC} PROPERTIES GENERATED TRUE)
add_library(gtest ${GTEST_ALL_SRC})
add_dependencies(gtest gtest_download)
target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(gtest
                           PUBLIC ${GTEST_INCLUDE_DIR}
                           PRIVATE ${source_dir})

# Disable some warnings for gtest source.
# - There are a lot of implicit conversions from signed to unsigned integers.
# - GTEST_DEFINE_STATIC_MUTEX_ does not initialize owner_ field. See
#   documentation for the macro in include/gtest/internal/gtest-port.h
if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options(gtest
                           PUBLIC -Wno-sign-compare
                           PRIVATE -Wno-missing-field-initializers)
endif()

set_source_files_properties(${GTEST_MAIN_SRC} PROPERTIES GENERATED TRUE)
add_library(gtest_main ${GTEST_MAIN_SRC})
add_dependencies(gtest_main gtest)
target_include_directories(gtest_main
                           PUBLIC ${GTEST_INCLUDE_DIR}
                           PRIVATE ${source_dir})

# GTEST_BOTH_LIBRARIES variable is set in this file to allow for some
# "compatibility" with the CMake FindGTest module. The idea being that it should
# be possible to just replace inclusion of this file with
# "find_package(GTest REQUIRED)" and vice versa and still have
# "target_link_libraries(foo ${GTEST_BOTH_LIBRARIES})" do the right thing
# (include directory is added implicitly etc).
set(GTEST_BOTH_LIBRARIES gtest gtest_main)
