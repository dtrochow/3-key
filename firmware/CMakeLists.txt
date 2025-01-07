cmake_minimum_required(VERSION 3.13)

set(project_name "3-key")

set(cmake_files ${CMAKE_CURRENT_LIST_DIR}/cmake)
set(src_dir ${CMAKE_CURRENT_LIST_DIR}/src)

set(PICO_BOARD waveshare_rp2040_plus_16mb)

include(${cmake_files}/pico_sdk_import.cmake)

project(${project_name} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_executable(${project_name}
    ${CMAKE_CURRENT_LIST_DIR}/3-key.cpp
)

target_include_directories(${project_name} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(buttons)
add_subdirectory(leds)
add_subdirectory(usb)
add_subdirectory(storage)
add_subdirectory(terminal)
add_subdirectory(keyscfg)
add_subdirectory(features)

target_link_libraries(${project_name} 
    pico_stdlib 
    pico_multicore
    buttons
    leds
    usb
    storage 
    terminal
    keyscfg
    features_handler
)

pico_add_extra_outputs(${project_name})
