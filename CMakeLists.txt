cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(rayni-staging)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

# TODO: option()(s?) and compiler checks etc... more warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

include(cmake/gtest.cmake)

enable_testing()

# TODO: Not fond of having a CMakeLists.txt in every directory. Not sure that
#       including from cmake/ is the way to go though.
include(cmake/lib.cmake)
