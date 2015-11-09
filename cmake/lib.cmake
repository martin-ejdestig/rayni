add_library(rayni
            src/lib/containers/fixed_size_stack.h
            src/lib/containers/variant.cpp
            src/lib/containers/variant.h
            src/lib/math/enum.h
            src/lib/math/hash.h)

target_include_directories(rayni
                           PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(librayni-unit_tests
               src/unit_tests/lib/containers/fixed_size_stack.cpp
               src/unit_tests/lib/containers/variant.cpp
               src/unit_tests/lib/math/enum.cpp
               src/unit_tests/lib/math/hash.cpp)

target_include_directories(librayni-unit_tests
                           PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(librayni-unit_tests
                      rayni
                      ${GTEST_BOTH_LIBRARIES})

add_test(librayni-unit_tests librayni-unit_tests)
