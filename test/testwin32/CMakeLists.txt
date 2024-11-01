
add_executable(testwin32 testwin32.cpp)
target_link_libraries(testwin32 trampoline winhttp)

add_test(NAME testwin32 COMMAND testwin32)
set_target_properties(testwin32 PROPERTIES FOLDER "tests")

