set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../../cmake/icm_build_failure_testing.cmake)
include(../../../cmake/icm_testing.cmake)

# Global include for icm based tests
include_directories("../../include")

file(GLOB test_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}" *.test.cpp)
message(test_files: ${test_files})

foreach(test_file ${test_files})
    get_filename_component(name_without_extension "${test_file}" NAME_WE)
    icm_add_test(
        NAME ${name_without_extension}
        SOURCES ${test_file})

    if(${test_file} MATCHES fail_*)
        set_tests_properties(${name_without_extension} PROPERTIES
            WILL_FAIL TRUE)
    endif()
endforeach()
