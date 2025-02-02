set(test_name "time_test")

set(TEST_SOURCES
    time_test.cpp
)
add_executable(${test_name} ${TEST_SOURCES})

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/time)

target_link_libraries(${test_name} PRIVATE time GTest::gtest_main)

gtest_discover_tests(${test_name})
