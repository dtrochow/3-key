cmake_minimum_required(VERSION 3.13)

set(project_name "3-key")

set(cmake_files ${CMAKE_CURRENT_LIST_DIR}/cmake)
set(src_dir ${CMAKE_CURRENT_LIST_DIR}/src)

set(PICO_BOARD waveshare_rp2040_plus_16mb)

set(PICO_USE_FASTEST_SUPPORTED_CLOCK 1)

include(${cmake_files}/pico_sdk_import.cmake)

project(${project_name} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# Check environment variable BUILD_TYPE and set CMAKE_BUILD_TYPE
if(DEFINED ENV{BUILD_TYPE})
    message(STATUS "Environment variable BUILD_TYPE is set to $ENV{BUILD_TYPE}")
    set(CMAKE_BUILD_TYPE $ENV{BUILD_TYPE} CACHE STRING "Build type" FORCE)
else()
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    message(STATUS "Environment variable BUILD_TYPE is not set. Defaulting to Debug.")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build type selected.")
    set(OPTIMIZATION_FLAG "-g -O0")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release build type selected.")
    set(OPTIMIZATION_FLAG "-O3")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Print the compiler being used
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

pico_sdk_init()

add_executable(${project_name}
    ${CMAKE_CURRENT_LIST_DIR}/3-key.cpp
)

target_include_directories(${project_name} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

# Define local libraries compile flags
set(LIBRARY_COMPILE_FLAGS
    "\
    -Werror\
    -Wall\
    -Wextra\
    -Wduplicated-cond\
    -Wnull-dereference\
    -Wduplicated-branches\
    -Wsign-conversion\
    -Wconversion\
    -Wshadow\
    -Wformat=2\
    -Wformat-extra-args\
    -Wpedantic\
    -Wfloat-equal\
    -Wunused\
    -Wuninitialized\
    -Wswitch-default\
    -Wswitch-enum\
    -Wcast-align\
    -Wstrict-aliasing\
    -Wstrict-overflow\
    -Wundef\
    -Wlogical-op\
    -Wmissing-declarations\
    -Wredundant-decls\
    -Wzero-as-null-pointer-constant\
    -Wnon-virtual-dtor\
    "
)

# Pass the variable to subdirectories
add_subdirectory(buttons)
add_subdirectory(leds)
add_subdirectory(usb)
add_subdirectory(storage)
add_subdirectory(terminal)
add_subdirectory(keyscfg)
add_subdirectory(features)
add_subdirectory(time)

add_compile_options(${project_name} PUBLIC ${OPTIMIZATION_FLAGS})

target_link_libraries(${project_name} PRIVATE
    pico_stdlib
    pico_multicore
    buttons
    leds
    usb
    storage
    terminal
    keyscfg
    features_handler
    time
)

pico_add_extra_outputs(${project_name})
