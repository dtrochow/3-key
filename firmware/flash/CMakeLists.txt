set(modulename "flash")
set(SOURCES 
        flash.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC include)
target_link_libraries(${modulename}
    pico_stdlib
    hardware_flash
    usb
)
