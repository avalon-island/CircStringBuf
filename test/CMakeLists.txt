cmake_minimum_required(VERSION 3.5)

project(circbuf_test C)

add_executable(circbuf_test
    circbuf_test.c)
include(CTest)
enable_testing()

target_link_libraries(circbuf_test unity)

add_library(unity STATIC ~/software/Unity/src/unity.c)
target_include_directories(unity PUBLIC ~/software/Unity/src)
 
add_test(circbuf_test circbuf_test.c)
