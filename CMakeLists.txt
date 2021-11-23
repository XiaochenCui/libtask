cmake_minimum_required(VERSION 3.10)

project(LIBTASK C)

set(C_STANDARD 99)

set(PROJECT_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

file(GLOB lib_files
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.h
)
add_library(task STATIC ${lib_files})

set(TARGET_LIST primes)
foreach (TARGET ${TARGET_LIST})
    add_executable(${TARGET} examples/${TARGET}.c)
    target_include_directories(${TARGET} PRIVATE src)
    target_link_libraries(${TARGET} task)
endforeach()