# Macro to define a Rust library integration
macro(define_rust_library LIB_NAME LIB_DIR LIB_TARGET)
    set(RUST_CRATE_DIR "${LIB_DIR}/src")
    set(CBINDGEN_CONFIG "${CMAKE_SOURCE_DIR}/cmake/cbindgen.toml")
    set(CBINDGEN_OUTPUT "${LIB_DIR}/inc/${LIB_NAME}.h")
    set(RUST_TARGET "${LIB_TARGET}")
    set(RUST_LIB_PATH "${LIB_DIR}/target/${RUST_TARGET}/release/lib${LIB_NAME}.a")
    file(GLOB_RECURSE RUST_SRC_FILES "${RUST_CRATE_DIR}/*.rs")

    add_custom_target(${LIB_NAME}_clean
        COMMAND cargo clean
        WORKING_DIRECTORY ${LIB_DIR}
        COMMENT "Cleaning Rust build artifacts for ${LIB_NAME}"
    )

    add_custom_command(
        OUTPUT ${CBINDGEN_OUTPUT}
        COMMAND ${CBINDGEN_EXECUTABLE} --config ${CBINDGEN_CONFIG} --crate ${LIB_NAME} --output ${CBINDGEN_OUTPUT}
        WORKING_DIRECTORY ${LIB_DIR}
        DEPENDS ${CBINDGEN_CONFIG} ${RUST_SRC_FILES}
        COMMENT "Generating C header for ${LIB_NAME} with cbindgen"
    )

    add_custom_target(${LIB_NAME}_header
        DEPENDS ${CBINDGEN_OUTPUT}
    )

    add_custom_command(
        OUTPUT ${RUST_LIB_PATH}
        COMMAND cargo build --release --target ${RUST_TARGET}
        WORKING_DIRECTORY ${LIB_DIR}
        DEPENDS ${LIB_DIR}/Cargo.toml ${RUST_SRC_FILES}
        COMMENT "Building Rust static library ${LIB_NAME} for ${RUST_TARGET}"
    )

    add_custom_target(${LIB_NAME}_lib
        DEPENDS ${LIB_NAME}_clean ${RUST_LIB_PATH}
    )

    add_library(${LIB_NAME} STATIC IMPORTED GLOBAL)
    set_target_properties(${LIB_NAME} PROPERTIES
        IMPORTED_LOCATION ${RUST_LIB_PATH}
        INTERFACE_INCLUDE_DIRECTORIES ${LIB_DIR}/inc
    )

    add_dependencies(${LIB_NAME} ${LIB_NAME}_header ${LIB_NAME}_lib)
endmacro()
