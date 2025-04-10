set(modulename "time")
set(SOURCES 
        time.cpp
)
add_library(${modulename} ${SOURCES})
target_include_directories(${modulename} PUBLIC include)

if(NOT UNIT_TEST)
    target_link_libraries(${modulename} pico_stdlib)
endif()

# Apply the library-specific compile flags
if(DEFINED LIBRARY_COMPILE_FLAGS)
    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "${LIBRARY_COMPILE_FLAGS}")
endif()
