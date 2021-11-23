cmake_minimum_required(VERSION 3.10)

project(LIBTASK C)

set(C_STANDARD 99)

set(PROJECT_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

add_library(task STATIC src/task.h src/task.c)

set(TARGET_LIST primes)
foreach (TARGET ${TARGET_LIST})
    add_executable(${TARGET} ${TARGET}.c)
    target_include_directories(${TARGET} PRIVATE src)
    target_link_libraries(${TARGET} task)
endforeach()