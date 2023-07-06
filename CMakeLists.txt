cmake_minimum_required(VERSION 3.25)

project(cr3_converter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(cr3_converter main.cpp src/ImageData.cpp src/ImageData.h)

add_subdirectory(LibRaw-cmake)

target_link_libraries(cr3_converter PRIVATE libraw::libraw)
target_compile_options(cr3_converter PRIVATE -Wall -Wextra -O -g)
