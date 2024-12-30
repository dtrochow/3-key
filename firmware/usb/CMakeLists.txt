set(modulename "usb")
set(SOURCES 
        hid.cpp
        tud.cpp
        cdc.cpp
        usb_descriptors.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC include)
target_link_libraries(${modulename}
    pico_stdlib 
    tinyusb_device 
    tinyusb_board
    buttons
)
