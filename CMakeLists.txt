# Top-level CMake configuration for YesRTOS.
# Configure through a preset, which selects the toolchain and target: see CMakePresets.json.
#   cmake --preset stm32f767
#   cmake --build --preset stm32f767

cmake_minimum_required(VERSION 3.21)

project(YesRTOS
        LANGUAGES C CXX ASM
        VERSION 1.0
)

# Target selection, normally set by the preset.
set(ARCH_DEFINED "" CACHE STRING "Target architecture: ARMV7M")
set_property(CACHE ARCH_DEFINED PROPERTY STRINGS ARMV7M)

if (NOT ARCH_DEFINED STREQUAL "ARMV7M")
  message(FATAL_ERROR "'ARCH_DEFINED' is not recognized: '${ARCH_DEFINED}'. Configure with a preset, e.g. 'cmake --preset stm32f767'.")
endif()

# Build Debug mode for gdb debugging.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

add_subdirectory(kernel)
add_subdirectory(app/multi_thread)
