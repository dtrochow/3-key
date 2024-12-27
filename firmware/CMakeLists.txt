cmake_minimum_required(VERSION 3.13)

set(project_name "3-key")

set(cmake_files ${CMAKE_CURRENT_LIST_DIR}/cmake)
set(src_dir ${CMAKE_CURRENT_LIST_DIR}/src)

include(${cmake_files}/pico_sdk_import.cmake)

project(${project_name} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_executable(${project_name}
    ${src_dir}/main.cpp
    ${src_dir}/usb_descriptors.cpp
    ${src_dir}/leds.cpp
    ${src_dir}/keys.cpp
)

pico_generate_pio_header(${project_name} ${cmake_files}/ws2812.pio)

target_include_directories(${project_name} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/inc
)

target_link_libraries(${project_name} 
    pico_stdlib 
    hardware_pio 
    tinyusb_device 
    tinyusb_board
    pico_multicore
)

pico_add_extra_outputs(${project_name})
