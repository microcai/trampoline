
add_compile_options(-m32)
set(CMAKE_EXE_LINKER_FLAGS "-m32")

set(CMAKE_SIZEOF_VOID_P 4)

add_compile_options(-ggdb)
add_link_options(-g)
