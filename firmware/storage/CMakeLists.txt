set(modulename "storage")
set(SOURCES 
        storage.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC include)
target_link_libraries(${modulename}
    pico_stdlib
    hardware_flash
)
