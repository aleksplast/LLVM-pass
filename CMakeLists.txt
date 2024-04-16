cmake_minimum_required(VERSION 3.0)
project(Pass VERSION 1.0.0)

set(PASS_DIR ${CMAKE_SOURCE_DIR}/pass)
set(VISUALIZER_DIR ${CMAKE_SOURCE_DIR}/visualizer)

set(PASS_SRC ${PASS_DIR}/pass.cpp)
add_library(Pass SHARED ${PASS_SRC})

set(VISUALIZER_SRC ${VISUALIZER_DIR}/main.cpp)

find_package(LLVM)
target_include_directories(Pass PUBLIC ${LLVM_INCLUDE_DIRS})

add_executable(visualizer)
target_sources(visualizer PRIVATE ${VISUALIZER_SRC})
