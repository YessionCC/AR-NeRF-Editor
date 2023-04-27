
cmake_minimum_required(VERSION 3.0)

project(cpp_socket)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11") 
set(CMAKE_BUILD_TYPE "Release")

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*)
include_directories(${PROJECT_SOURCE_DIR}/include)


add_library(cppSocket ${SRC_FILES})
