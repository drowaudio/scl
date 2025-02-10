set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../cmake/icm_build_failure_testing.cmake)
include(../../cmake/icm_testing.cmake)

# Global include for icm based tests
include_directories("../include")

file(GLOB_RECURSE pass_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}" *.test.cpp)
message(pass_files: ${pass_files})

foreach(pass_file ${pass_files})
    get_filename_component(name_without_extension "${pass_file}" NAME_WE)
    icm_add_test(
        NAME ${name_without_extension}
        SOURCES ${pass_file}
    )
endforeach()
