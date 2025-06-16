# CMake version.
cmake_minimum_required(VERSION 3.10)

# Cross compilation tool chain.
include(${CMAKE_CURRENT_LIST_DIR}/gcc-arm-none-eabi.cmake)

set(PROJECT_NAME YesRTOSMachineLayer)
project(${PROJECT_NAME})

 # Enable C, C++, and ASM languages for the project.
enable_language(C CXX ASM)

# Add library which includes all other bare metal code.
add_library(LibBareMetal
    ${CMAKE_CURRENT_LIST_DIR}/startup_stm32f767xx.s
    ${CMAKE_CURRENT_LIST_DIR}/trace.c
    ${CMAKE_CURRENT_LIST_DIR}/atomic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/timeslice.cpp
)
target_include_directories(LibBareMetal PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}   # ${CMAKE_CURRENT_LIST_DIR} maps to current folder: arch/armv7m
    ${CMAKE_CURRENT_LIST_DIR}/../../include
)

target_compile_definitions(LibBareMetal PRIVATE ARMV7M)
