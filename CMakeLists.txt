cmake_minimum_required(VERSION 3.27)
project(comp_grafica_2)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(comp_grafica_2
        src/main.cpp
        src/math3D.cpp
        src/math3D.h
        src/render.cpp
        src/render.h
        src/solidos.cpp
        src/solidos.h
        src/ui.cpp
        src/ui.h)
