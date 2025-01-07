set(modulename "ctrl_c_v")
set(SOURCES 
        ctrl_c_v.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC include)
target_link_libraries(${modulename}
    pico_stdlib 
    features_handler
    usb
)
