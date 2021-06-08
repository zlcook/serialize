cmake_minimum_required(VERSION 3.15)

project(serialize LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(AFTER src ${CMAKE_CURRENT_BINARY_DIR}/src)

set(CMAKE_BUILD_TYPE "Debug")

enable_testing()

add_subdirectory(src)
