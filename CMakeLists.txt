# start cmake with the following command:
#
# mkdir build
# cmake -Bbuild .
# cmake --build build
#
cmake_minimum_required(VERSION 3.20)

project(playground)

find_package(OpenGL REQUIRED)

add_executable(playground
    glad.c
    glad_wgl.c
    openglapp.cpp
    openglapp.hpp
    program.cpp
    shader.cpp
    shader.hpp
    vertexbuffer.hpp
)

target_include_directories(playground
    PRIVATE
        include
)

# add dependencies
include(cmake/CPM.cmake)

CPMAddPackage("gh:gabime/spdlog#v1.15.0")
CPMAddPackage("gh:g-truc/glm#0.9.9.8")

target_link_libraries(playground
    PRIVATE
        OpenGL::GL
        spdlog
        glm
)

target_compile_features(playground
    PRIVATE
        cxx_std_20
)
