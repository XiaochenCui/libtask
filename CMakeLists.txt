cmake_minimum_required(VERSION 3.10)

project(LIBTASK C)

include(CMakeDependentOption)
include(CheckCCompilerFlag)

set(C_STANDARD 89)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(UNIX)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g ")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fomit-frame-pointer ")
endif()

message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")

set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}/lib)

file(GLOB lib_files
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.h
)
add_library(task STATIC ${lib_files})

set(TARGET_LIST testdelay primes helloworld chan_1 chan_2 chan_3)
foreach (TARGET ${TARGET_LIST})
    add_executable(${TARGET} examples/${TARGET}.c)
    target_include_directories(${TARGET} PRIVATE src)
    target_link_libraries(${TARGET} task)
endforeach()
