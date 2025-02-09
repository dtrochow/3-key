set(test_name "terminal_test")

set(TEST_SOURCES
    terminal_binary_mode_test.cpp
)
add_executable(${test_name} ${TEST_SOURCES})

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/terminal)

target_link_libraries(${test_name} PRIVATE terminal GTest::gtest_main)

gtest_discover_tests(${test_name})
