# CMake version.
cmake_minimum_required(VERSION 3.10)

# Cross compilation tool chain.
include(${CMAKE_CURRENT_LIST_DIR}/gcc-riscv-none-eabi.cmake)

set(PROJECT_NAME YesRTOSMachineLayer)
project(${PROJECT_NAME})

 # Enable C, C++, and ASM languages for the project.
enable_language(C CXX ASM)

# Add library which includes all other bare metal code.
add_library(LibBareMetal
    ${CMAKE_CURRENT_LIST_DIR}/startup_rv32i.s
)

target_include_directories(LibBareMetal PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/../../include
)
