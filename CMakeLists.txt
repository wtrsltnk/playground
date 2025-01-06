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
    include/KHR/khrplatform.h
    include/glad/glad.h
    include/glad/glad_wgl.h
    src/glad.c
    src/glad_wgl.c
    src/openglapp.cpp
    src/openglapp.hpp
    src/program.cpp
    src/shader.cpp
    src/shader.hpp
    src/vertexbuffer.hpp
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
