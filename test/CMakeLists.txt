add_custom_target(tests)
set_target_properties(tests PROPERTIES FOLDER "tests")

add_subdirectory(test1)

if (WIN32)
add_subdirectory(testwin32)
endif()