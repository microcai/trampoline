
add_executable(test2 test2.cpp)
target_link_libraries(test2 trampoline)

add_test(NAME test2 COMMAND test2)
set_target_properties(test2 PROPERTIES FOLDER "tests")

if (APPLE)
target_link_options(test2 PRIVATE
    LINKER:-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif()