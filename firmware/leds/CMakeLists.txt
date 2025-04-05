set(modulename "leds")
set(SOURCES 
        leds.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC
        include
        ${CMAKE_CURRENT_BINARY_DIR}
)

pico_generate_pio_header(${project_name} ${CMAKE_CURRENT_LIST_DIR}/include/ws2812.pio)

target_link_libraries(${modulename}
    hardware_pio 
    buttons
    keyscfg
)

# Apply the library-specific compile flags
if(DEFINED LIBRARY_COMPILE_FLAGS)
    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "${LIBRARY_COMPILE_FLAGS}")
endif()
