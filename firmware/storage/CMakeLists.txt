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

# Apply the library-specific compile flags
if(DEFINED LIBRARY_COMPILE_FLAGS)
    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "${LIBRARY_COMPILE_FLAGS}")
endif()
