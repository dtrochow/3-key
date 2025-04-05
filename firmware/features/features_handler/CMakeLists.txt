set(modulename "features_handler")
set(SOURCES 
        features_handler.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC include)
target_link_libraries(${modulename}
    pico_stdlib 
    buttons
    ctrl_c_v
    time_tracker
    time
)

# Apply the library-specific compile flags
if(DEFINED LIBRARY_COMPILE_FLAGS)
    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "${LIBRARY_COMPILE_FLAGS}")
endif()
