cmake_minimum_required(VERSION 3.15)
project(software_renderer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIR})

add_executable(software_renderer matrix.h model.h model.cpp renderer.h renderer.cpp main.cpp)
target_link_libraries(software_renderer ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

add_custom_command(TARGET software_renderer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:software_renderer>/data/)