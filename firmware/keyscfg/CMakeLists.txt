set(modulename "keyscfg")
add_library(${modulename})
target_include_directories(${modulename} PUBLIC include)
target_link_libraries(${modulename}
    pico_stdlib 
    leds
    buttons
)
