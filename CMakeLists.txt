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
option(QEMU "Build for QEMU netduinoplus2 (Cortex-M4F) instead of STM32F767 (Cortex-M7)" OFF)

# Core clock of the board, which SysTick counts: QEMU runs netduinoplus2 (STM32F405) at 168 MHz.
if (QEMU)
  add_compile_definitions(CPU_CLK_FREQ_HZ=168000000UL)
endif()

if (NOT ARCH_DEFINED STREQUAL "ARMV7M")
  message(FATAL_ERROR "'ARCH_DEFINED' is not recognized: '${ARCH_DEFINED}'. Configure with a preset, e.g. 'cmake --preset stm32f767'.")
endif()

# Optional timeslice override, e.g. to stress preemption on QEMU (see the qemu-stress preset).
set(TIMESLICE_FREQ_HZ "" CACHE STRING "Override TIMESLICE_FREQ_HZ in config.h (Hz); empty keeps the default")
if (TIMESLICE_FREQ_HZ)
  add_compile_definitions(TIMESLICE_FREQ_HZ=${TIMESLICE_FREQ_HZ}UL)
endif()

# Build Debug mode for gdb debugging.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

add_subdirectory(kernel)
add_subdirectory(app/multi_thread)

# QEMU regression tests: ctest --preset qemu
if (QEMU)
  enable_testing()
  add_subdirectory(tests/qemu)
endif()
