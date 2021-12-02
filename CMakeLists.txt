cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "gcc-9")

project(LIBTASK C)

set(C_STANDARD 99)

# -g for debugging
set(CMAKE_C_FLAGS "-g")

set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}/lib)

file(GLOB lib_files
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.h
)
add_library(task STATIC ${lib_files})

set(TARGET_LIST primes tcpproxy helloworld chan_1 chan_2 chan_3)
foreach (TARGET ${TARGET_LIST})
    add_executable(${TARGET} examples/${TARGET}.c)
    target_include_directories(${TARGET} PRIVATE src)
    target_link_libraries(${TARGET} task)
endforeach()