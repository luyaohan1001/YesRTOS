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

# Board, normally set by the preset. A board fixes the CPU (toolchain file), its core clock, memory layout and trace
# output (kernel/arch/armv7m/CMakeLists.txt), and for a QEMU board the machine the images run on.
set(BOARD "stm32f767" CACHE STRING "Target board: stm32f767 (hardware), netduinoplus2 or ast1030 (QEMU)")
set_property(CACHE BOARD PROPERTY STRINGS stm32f767 netduinoplus2 ast1030)
if (BOARD STREQUAL "stm32f767")
  set(YESRTOS_QEMU_MACHINE "")
elseif (BOARD STREQUAL "netduinoplus2")
  set(YESRTOS_QEMU_MACHINE "netduinoplus2")
elseif (BOARD STREQUAL "ast1030")
  set(YESRTOS_QEMU_MACHINE "ast1030-evb")
else()
  message(FATAL_ERROR "'BOARD' is not recognized: '${BOARD}'. Configure with a preset, e.g. 'cmake --preset stm32f767'.")
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
add_subdirectory(drivers)
add_subdirectory(app/multi_thread)
add_subdirectory(app/stack_monitor)

# QEMU regression tests: ctest --preset qemu
if (YESRTOS_QEMU_MACHINE)
  enable_testing()
  add_subdirectory(tests/qemu)
endif()
