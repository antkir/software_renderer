cmake_minimum_required(VERSION 3.15)
project(software_renderer)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(sr renderer.h renderer.cpp main.cpp)
target_link_libraries(sr ${SDL2_LIBRARIES})