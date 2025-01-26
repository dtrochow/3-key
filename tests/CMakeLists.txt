cmake_minimum_required(VERSION 3.14)
project(time_unit_tests)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# GoogleTest setup
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(FIRMWARE_PATH ${CMAKE_CURRENT_LIST_DIR}/../firmware)

# -------------------------------------------------------------------------- #
#                              Test Executables                              #
# -------------------------------------------------------------------------- #

add_executable(time_test
  ${FIRMWARE_PATH}/time/test/time_test.cpp
)

# -------------------------------------------------------------------------- #
#                                  Libraries                                 #
# -------------------------------------------------------------------------- #

add_subdirectory(../firmware/time ${CMAKE_CURRENT_BINARY_DIR}/firmware_time)

target_link_libraries(time_test
  time
  gtest_main
)

# -------------------------------------------------------------------------- #
#                                    Tests                                   #
# -------------------------------------------------------------------------- #

add_test(NAME time_test COMMAND time_test)
