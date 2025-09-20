cmake_minimum_required(VERSION 3.16)

project(math_example C)

# Find cbindgen (assumes it's installed and in PATH)
find_program(CBINDGEN_EXECUTABLE cbindgen REQUIRED)

# Set paths
set(RUST_CRATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(CBINDGEN_CONFIG "${CMAKE_SOURCE_DIR}/../cbindgen.toml")
set(CBINDGEN_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/inc/math_example.h")
set(RUST_LIB_NAME "math_example")
set(RUST_TARGET "thumbv6m-none-eabi")
set(RUST_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/target/${RUST_TARGET}/release/lib${RUST_LIB_NAME}.a")
file(GLOB_RECURSE RUST_SRC_FILES "${RUST_CRATE_DIR}/*.rs")
# Generate C header from Rust crate
add_custom_target(math_example_clean
    COMMAND cargo clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Cleaning Rust build artifacts with cargo clean"
)

add_custom_command(
    OUTPUT ${CBINDGEN_OUTPUT}
    COMMAND ${CBINDGEN_EXECUTABLE} --clean --config ${CBINDGEN_CONFIG} --crate ${RUST_LIB_NAME} --output ${CBINDGEN_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CBINDGEN_CONFIG} ${RUST_CRATE_DIR} ${RUST_SRC_FILES}
    COMMENT "Generating C header with cbindgen"
)

add_custom_target(math_example_header
    DEPENDS ${CBINDGEN_OUTPUT}
)

# Build Rust static library
add_custom_command(
    OUTPUT ${RUST_LIB_PATH}
    COMMAND cargo build --release --target ${RUST_TARGET}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml ${RUST_CRATE_DIR}
    COMMENT "Building Rust static library for ARM and running ranlib"
)

add_custom_target(math_example_lib
    DEPENDS math_example_clean ${RUST_LIB_PATH}
)

# Create CMake interface library for linking
add_library(math_example STATIC IMPORTED GLOBAL)
set_target_properties(math_example PROPERTIES
    IMPORTED_LOCATION ${RUST_LIB_PATH}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# Ensure header and library are built before linking
add_dependencies(math_example math_example_header math_example_lib)
