
add_executable(test1 test1.cpp)
target_link_libraries(test1 trampoline)

add_test(NAME test1 COMMAND test1)
set_target_properties(test1 PROPERTIES FOLDER "tests")