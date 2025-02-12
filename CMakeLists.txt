cmake_minimum_required(VERSION 3.28)
project(sync_send)

set (CMAKE_CXX_STANDARD 23)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-return=runtime")

enable_testing()
add_subdirectory(tests)
add_subdirectory(include/scl)
add_subdirectory(include/scl/utils)