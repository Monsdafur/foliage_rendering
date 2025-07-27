cmake_minimum_required(VERSION 3.28)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--quiet")
endif()

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)

add_subdirectory(external/glad)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/stb)
add_subdirectory(external/imgui)

add_library(foliage_render STATIC 
            src/shader.cpp
            src/texture.cpp
            src/window.cpp
            src/renderer.cpp
            src/mesh.cpp
            src/chunk.cpp
            )
target_include_directories(foliage_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(foliage_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/PerlinNoise)

target_link_libraries(imgui PRIVATE glfw glad OpenGL::GL)
target_link_libraries(foliage_render PUBLIC stb glfw glad glm OpenGL::GL imgui)

add_executable(grass_field main.cpp)
target_link_libraries(grass_field PUBLIC foliage_render)