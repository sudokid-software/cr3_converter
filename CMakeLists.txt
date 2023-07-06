cmake_minimum_required(VERSION 3.25)
project(cr3_converter)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Add link for libraw
link_libraries(raw)

add_executable(cr3_converter main.cpp src/ImageData.cpp src/ImageData.h)
