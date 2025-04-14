cmake_minimum_required(VERSION 3.16)

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../include ../include/Filesystem ../include/Parser)
link_directories(../lib)

file(GLOB SOURCES ../src/*.cpp ../include/Filesystem/*.cpp ../include/Parser/*.cpp)

add_executable(main ${SOURCES})
target_compile_options(main PRIVATE -Wall -Ofast)
target_link_libraries(main PRIVATE -ltiff)
