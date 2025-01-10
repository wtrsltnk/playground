# start cmake with the following command:
#
# mkdir build
# cmake -Bbuild .
# cmake --build build
#
cmake_minimum_required(VERSION 3.20)

include(cmake/CPM.cmake)

project(playground)

option(BUILD_EXAMPLE "Build the example player" OFF)

add_library(playground
    include/KHR/khrplatform.h
    include/glad/glad.h
    include/glad/glad_wgl.h
    include/openglapp.hpp
    include/shader.hpp
    include/vertexbuffer.hpp
)

target_sources(playground
    PRIVATE
        src/glad.c
        src/glad_wgl.c
        src/openglapp.cpp
        src/shader.cpp
)

find_package(OpenGL REQUIRED)

CPMAddPackage("gh:gabime/spdlog#v1.15.0")

CPMAddPackage("gh:g-truc/glm#0.9.9.8")

target_link_libraries(playground
    PUBLIC
        OpenGL::GL  # Needed for wglCreateContext, wglDeleteContext and wglMakeCurrent
        spdlog
        glm
)

target_compile_features(playground
    PUBLIC
        cxx_std_20
)

target_include_directories(playground
    PUBLIC
        include
)

if (BUILD_EXAMPLE)
    add_executable(playground-player
        src/program.cpp
    )

    target_link_libraries(playground-player
        PRIVATE
            playground
    )

    target_compile_features(playground-player
        PRIVATE
            cxx_std_20
    )
endif(BUILD_EXAMPLE)
