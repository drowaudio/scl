set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../cmake/icm_build_failure_testing.cmake)
include(../cmake/icm_testing.cmake)

# Global include for icm based tests
include_directories("../include")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
    OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set (ERROR_STRING "does not satisfy 'send'")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (ERROR_STRING "constraints not satisfied")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
endif()
    
icm_glob_build_failure_tests(
    PATTERN fail_*
    ERROR_MATCHES ${ERROR_STRING}
)

file(GLOB_RECURSE pass_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}" pass_*.cpp)
message(pass_files: ${pass_files})

foreach(pass_file ${pass_files})
    get_filename_component(name_without_extension "${pass_file}" NAME_WE)
    icm_add_test(
        NAME ${name_without_extension}
        SOURCES ${pass_file}
    )
endforeach()
