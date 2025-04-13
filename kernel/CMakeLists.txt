# CMake configuration for YesRTOS kernel build.

# CMake version.
cmake_minimum_required(VERSION 3.10)

# Cross compilation tool chain.
include(${CMAKE_CURRENT_SOURCE_DIR}/arch/armv7m/gcc-arm-none-eabi.cmake)

# Setup project and source.
set(PROJECT_NAME YesRTOSKernel)
project(${PROJECT_NAME}
        LANGUAGES C CXX ASM
        VERSION 1.0
)

# Specify C standard.
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Specify Cpp standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Build Debug mode for gdb debugging.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Enable compile command export to json (for CMake debugging).
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Define static library for YesRTOSKernel.
set(LIBRARY_NAME LibYesRTOSKernel)

add_library(${LIBRARY_NAME} STATIC
  ${CMAKE_CURRENT_LIST_DIR}/src/rr_scheduler.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/preempt_fifo_scheduler.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/thread.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/bitops.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/mempool.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/linkedlist.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/test.cpp
)

target_include_directories(${LIBRARY_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include/
    ${CMAKE_CURRENT_LIST_DIR}/arch/armv7m/
)

# Include subdirectory to build sub-component together as part of the kernel.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/arch/armv7m)

# Define using arch armv7m
add_definitions(-DARCH_ARMV7M)

# Flash command.
add_custom_target(flash
COMMAND openocd
            -f interface/stlink-v2-1.cfg
            -f target/stm32f7x.cfg
            -c 'program ${CMAKE_CURRENT_LIST_DIR}/build/${PROJECT_NAME}.elf verify reset exit'
)

# Dump symbol table using nm util.
add_custom_target(dump_symbol_table
        arm-none-eabi-nm
        --demangle
        --size-sort
        --print-size
        --numeric-sort
        --line-numbers
        --radix=decimal
        YesRTOS.elf > symtable.dump
)

# Dump size from ELF (executable and linkable format) file.
add_custom_target(dump_section_size
        arm-none-eabi-size -A
        YesRTOS.elf > size.dump
)