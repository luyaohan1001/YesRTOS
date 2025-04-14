# CMake configuration for YesRTOS kernel build.

# CMake version.
cmake_minimum_required(VERSION 3.10)

# Cross compilation tool chain.
include(${CMAKE_CURRENT_SOURCE_DIR}/arch/armv7m/gcc-arm-none-eabi.cmake)

# Linker script for address-related symbol resolution (stack start address, etc.)
set(LINKER_SCRIPT_PATH ${CMAKE_CURRENT_LIST_DIR}/arch/armv7m/STM32F767ZITx_FLASH.ld)
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T ${LINKER_SCRIPT_PATH}")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -T \"${LINKER_SCRIPT_PATH}\"")

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
  ${CMAKE_CURRENT_LIST_DIR}/src/mutex.cpp
)

target_include_directories(${LIBRARY_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include/
    ${CMAKE_CURRENT_LIST_DIR}/arch/armv7m/
)

# Include subdirectory to build sub-component together as part of the kernel.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/arch/armv7m)

# Define using arch armv7m
add_definitions(-DARCH_ARMV7M)