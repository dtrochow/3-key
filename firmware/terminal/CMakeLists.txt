set(modulename "terminal")
set(SOURCES 
        terminal.cpp
        binary_mode.cpp
        text_mode.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC include)
target_link_libraries(${modulename}
        pico_stdlib
        storage
        keyscfg
        features_handler
        time
)
