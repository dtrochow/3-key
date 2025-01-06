set(modulename "time-tracker")
set(SOURCES 
        time_tracker.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC include)
target_link_libraries(${modulename}
    pico_stdlib 
)
