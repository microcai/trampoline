cmake_minimum_required(VERSION 3.25)
project(trampoline CXX ASM)

set(CMAKE_CXX_STANDARD 20)

set(ASM_SRC "")

if (MSVC)
enable_language(CXX C ASM_MASM)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)

list(APPEND ASM_SRC src/x86_64.asm)

else()

list(APPEND ASM_SRC src/x86.asm)

endif()

add_link_options(/INCREMENTAL:NO)
add_link_options(/SAFESEH:NO)

endif(MSVC)

message(STATUS "compiler id is" ${CMAKE_CXX_COMPILER_ID})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)

list(APPEND ASM_SRC src/x86_64.S)

else()

list(APPEND ASM_SRC src/x86.S)

endif()

endif()

add_library(trampoline src/executable_allocator.cpp src/trampoline.cpp ${ASM_SRC})

target_include_directories(trampoline PUBLIC include)

add_subdirectory(test)

