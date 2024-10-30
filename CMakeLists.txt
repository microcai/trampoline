cmake_minimum_required(VERSION 3.25)
project(trampoline CXX ASM)

set(CMAKE_CXX_STANDARD 20)

add_library(trampoline src/executable_allocator.cpp src/trampoline.cpp src/x86_64.gas)

target_include_directories(trampoline PUBLIC include)

add_subdirectory(test)

