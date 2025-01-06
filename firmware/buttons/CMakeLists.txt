set(modulename "buttons")
set(SOURCES 
        buttons.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC include)
target_link_libraries(${modulename}
    pico_stdlib 
    tinyusb_board
    tinyusb_device
    usb
    leds
    keyscfg
)
