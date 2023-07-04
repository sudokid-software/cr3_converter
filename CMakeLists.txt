cmake_minimum_required(VERSION 3.25)

project(cr3_converter)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SOURCE_FILE src/*.cpp)
add_executable(cr3_converter ${SOURCE_FILE})

target_include_directories(cr3_converter PRIVATE include)
target_compile_options(cr3_converter PRIVATE -Wall -Wextra -O -g)
