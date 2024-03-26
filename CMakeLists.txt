cmake_minimum_required(VERSION 3.0)
project(Pass VERSION 1.0.0)

set(PASS_DIR ${CMAKE_SOURCE_DIR}/pass)
set(EXAMPLE_DIR ${CMAKE_SOURCE_DIR}/examples)

set(PASS_SRC ${PASS_DIR}/pass.cpp)
add_library(Pass SHARED ${PASS_SRC})

set(EXAMPLE_SRC ${EXAMPLE_DIR}/visualise_example.cpp)
add_executable(visualisator)
target_sources(visualisator PRIVATE ${EXAMPLE_SRC})
