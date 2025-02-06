set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_subdirectory(icb_tests)

include(../cmake/icm_build_failure_testing.cmake)
include(../cmake/icm_testing.cmake)

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

return()

add_custom_target(all_tests)

#======================================
message(CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE test_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}" pass_*.cpp fail_*.cpp)
message(test_files: ${test_files})

#======================================
function(try_compile_test test_name source_file)
    # Create a temporary directory for the try_compile
    set(tmp_dir "${CMAKE_BINARY_DIR}/CMakeFiles/${test_name}_dir")
    file(REMOVE_RECURSE ${tmp_dir})
    file(MAKE_DIRECTORY ${tmp_dir})

    if(${test_name} MATCHES "fail")
        set(should_fail true)
    else()
        set(should_fail false)
    endif()

    # Try to compile the source file
    try_compile(result ${tmp_dir}
        ${CMAKE_CURRENT_SOURCE_DIR}
        SOURCES ${source_file}
        CXX_STANDARD 23
        OUTPUT_VARIABLE output)

    # Check the result and define the test accordingly
    if (NOT result)
        message(STATUS "Test '${test_name}' failed to compile")
        if (${should_fail})
            message(STATUS "✅")
        else()
            message(STATUS "❌")
        endif()

        add_test(NAME ${test_name} COMMAND ${CMAKE_COMMAND} -E false)
    else()
        message(STATUS "Test '${test_name}' compiled sucessfully")
        if (${should_fail})
            message(STATUS "❌")
        else()
            message(STATUS "✅")
        endif()

        add_test(NAME ${test_name} COMMAND ${CMAKE_COMMAND} -E true)
    endif()

    if(${should_fail})
        set_property(TEST ${test_name} PROPERTY WILL_FAIL true)
    endif()

    set_tests_properties(${test_name} PROPERTIES LABELS "${output}") # Add as label to see in test run log
    file(REMOVE_RECURSE ${tmp_dir})

    # Print compilation output for debugging
#    message(STATUS "Compilation Output:\n${output}")
endfunction()


foreach(file ${test_files})
    message("\n----------------------------------------------------------------------------------------------------")
    message(STATUS "Processing file: ${file}")
    get_filename_component(test_name ${file} NAME_WLE)
    message(STATUS "Creating test: ${test_name}")

    add_executable(${test_name}
        ${file})

#    if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
#        target_link_libraries(${test_name}
#            atomic)
#    endif()

    try_compile_test (${test_name} ${file})

#    if(${test_name} MATCHES "fail")
#        message(STATUS "TEST SHOULD FAIL")
#        set_property(TEST ${test_name} PROPERTY WILL_FAIL true)
#    endif()

    add_dependencies(all_tests ${test_name})

endforeach()
