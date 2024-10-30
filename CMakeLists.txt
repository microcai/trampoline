cmake_minimum_required(VERSION 3.25)
project(trampoline CXX)

enable_language(CXX C ASM_MASM)
set(CMAKE_CXX_STANDARD 20)

set(ASM_SRC "")

if (MSVC)

list(APPEND ASM_SRC src/x86_64.asm)

add_compile_options(/INCREMENTAL:NO)

endif(MSVC)

add_library(trampoline src/executable_allocator.cpp src/trampoline.cpp ${ASM_SRC})

target_include_directories(trampoline PUBLIC include)

add_subdirectory(test)

