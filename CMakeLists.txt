cmake_minimum_required(VERSION 3.0)
project(Pasclang VERSION 0.0.1 LANGUAGES CXX)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

SET(CMAKE_CXX_FLAGS "-std=c++1z -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable ${CMAKE_CXX_FLAGS}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
include_directories("include/")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(-DPASCLANG_RT_BUILD_PATH="${CMAKE_BINARY_DIR}/lib")
add_definitions(-DPASCLANG_RT_INSTALL_PATH="${CMAKE_INSTALL_PREFIX}/lib")

if(DEFINED PASCLANG_LINKER_DRIVER)
    message(STATUS "Using provided object linker driver ${PASCLANG_LINKER_DRIVER}")
    add_definitions(-DPASCLANG_LINKER_DRIVER="${PASCLANG_LINKER_DRIVER}")
else(DEFINED PASCLANG_LINKER_DRIVER)
    add_definitions(-DPASCLANG_LINKER_DRIVER="${CMAKE_CXX_COMPILER}")
    message(STATUS "Using default system linker driver ${CMAKE_CXX_COMPILER}")
    message(STATUS "To change linker driver, set the CMake variable PASCLANG_LINKER_DRIVER to the driver's path")
endif(DEFINED PASCLANG_LINKER_DRIVER)

add_subdirectory(src)
add_subdirectory(rt)
add_subdirectory(test)

