cmake_minimum_required(VERSION 3.28)
project(sync_send)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-return=runtime")

add_executable(sync_send
    main.cpp
    safe_thread.h
    sync_send.h
    syncronized_value.h
    type_traits.test.cpp)

target_compile_options(sync_send PUBLIC
    -Wall -Wextra -Wpedantic)

enable_testing()
add_subdirectory(tests)