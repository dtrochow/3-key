cmake_minimum_required(VERSION 3.16)

set(project_name "math_example")

project(${project_name} C)

define_rust_library(${project_name} "${CMAKE_CURRENT_SOURCE_DIR}" "thumbv6m-none-eabi")
