
cmake_minimum_required(VERSION 3.0)

project(ycrr)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++17") 
set(CMAKE_BUILD_TYPE "Release")

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*)
include_directories(${PROJECT_SOURCE_DIR}/include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 

set(EXE_FILE main)

add_executable(${EXE_FILE} ${PROJECT_SOURCE_DIR}/test/${EXE_FILE}.cpp ${SRC_FILES})

include_directories(${PROJECT_SOURCE_DIR}/include/imgui)
add_subdirectory(${PROJECT_SOURCE_DIR}/include/imgui)

include_directories(${PROJECT_SOURCE_DIR}/include/cpp_socket/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/include/cpp_socket)

target_link_libraries(${EXE_FILE} 
  cppSocket imgui 
  glfw X11 Xxf86vm Xrandr pthread 
  Xi Xinerama GL GLU dl assimp
)
