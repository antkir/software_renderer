cmake_minimum_required(VERSION 3.10)
project(software_renderer)

set(CMAKE_CXX_STANDARD 17)

function(deploy_shared_library NAME OUTPUT_DIRECTORY WINDOWS_PATH DARWIN_PATH LINUX_PATH)
    set(input "")
    if(WIN32)
        set(input "${WINDOWS_PATH}")
    elseif(APPLE)
        set(input "${DARWIN_PATH}")
    else()
        set(input "${LINUX_PATH}")
    endif()

    get_filename_component(file_name "${input}" NAME)
    set(output "${OUTPUT_DIRECTORY}/${file_name}")

    if(NOT UNIX)
        file(TO_NATIVE_PATH "${input}" input)
        file(TO_NATIVE_PATH "${output}" output)
    endif()

    add_custom_command(
            DEPENDS "${input}"
            OUTPUT "${output}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${input}" "${output}"
            COMMENT "Deploying ${NAME}"
    )

    add_custom_target("deploy_${NAME}" ALL DEPENDS "${output}")
endfunction()

file(GLOB_RECURSE SOFTWARE_RENDERER_SRC "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(software_renderer ${SOFTWARE_RENDERER_SRC})

target_include_directories(software_renderer PRIVATE "${CMAKE_SOURCE_DIR}/3rdparty/tinyobj/")
target_include_directories(software_renderer PRIVATE "${CMAKE_SOURCE_DIR}/3rdparty/SDL2/${CMAKE_HOST_SYSTEM_NAME}/include/")

if(WIN32)
    target_link_libraries(software_renderer PRIVATE "${CMAKE_SOURCE_DIR}/3rdparty/SDL2/Windows/lib/SDL2main.lib")
    target_link_libraries(software_renderer PRIVATE "${CMAKE_SOURCE_DIR}/3rdparty/SDL2/Windows/lib/SDL2.lib")
elseif(APPLE)
    target_link_libraries(software_renderer PRIVATE "${CMAKE_SOURCE_DIR}/3rdparty/SDL2/Darwin/bin/libSDL2.dylib")
elseif(UNIX)
    target_link_libraries(software_renderer PRIVATE "${CMAKE_SOURCE_DIR}/3rdparty/SDL2/Linux/bin/libSDL2.so")
endif()

add_custom_command(TARGET software_renderer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:software_renderer>/data/)

deploy_shared_library("SDL2"
        "${CMAKE_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/3rdparty/SDL2/Windows/bin/SDL2.dll"
        "${CMAKE_SOURCE_DIR}/3rdparty/SDL2/Darwin/bin/libSDL2.dylib"
        "${CMAKE_SOURCE_DIR}/3rdparty/SDL2/Linux/bin/libSDL2.so"
        )

add_dependencies(software_renderer deploy_SDL2)