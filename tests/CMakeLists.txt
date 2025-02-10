set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../cmake/icm_build_failure_testing.cmake)
include(../cmake/icm_testing.cmake)

# Global include for icm based tests
include_directories("../include")

icm_glob_build_failure_tests(
    PATTERN fail_*
    ERROR_MATCHES "does not satisfy 'send'"
)

# icm_add_test(
#   NAME mylib-core
#   LIBRARIES mylib
#   SOURCES
#       mylib-test-helpers.cpp
#       mylib-core-tests.cpp
# )

file(GLOB_RECURSE pass_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}" pass_*.cpp)
message(pass_files: ${pass_files})

foreach(pass_file ${pass_files})
    get_filename_component(name_without_extension "${pass_file}" NAME_WE)
    icm_add_test(
        NAME ${name_without_extension}
        SOURCES ${pass_file}
    )
endforeach()

#icm_add_build_failure_test(
#    NAME fail_empty_lambda_p
#    TARGET icm_fail_empty_lambda_p
#    SOURCES ../fail_empty_lambda_p.cpp
#)
