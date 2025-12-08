# CMake configuration for YesRTOS kernel build.

# CMake version.
cmake_minimum_required(VERSION 3.10)

# Cross compilation tool chain.
if (ARCH_DEFINED STREQUAL "ARMV7M")
  include(${CMAKE_CURRENT_SOURCE_DIR}/arch/armv7m/gcc-arm-none-eabi.cmake)
elseif (ARCH_DEFINED STREQUAL "RV32I")
  include(${CMAKE_CURRENT_SOURCE_DIR}/arch/rv32i/gcc-riscv-none-eabi.cmake)
else()
  message("'ARCH_DEFINED' is not recognized: ${ARCH_DEFINED}")
endif()


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

# Define using arch armv7m
add_library(${LIBRARY_NAME} STATIC
  ${CMAKE_CURRENT_LIST_DIR}/src/rr_scheduler.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/preempt_fifo_scheduler.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/thread.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/atomic_section.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/mempool.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/linkedlist.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/spinlock.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/mutex.cpp
)

if (ARCH_DEFINED STREQUAL "ARMV7M")
  target_compile_definitions(${LIBRARY_NAME} PRIVATE ARMV7M)

  target_include_directories(${LIBRARY_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/arch/armv7m/
    ${CMAKE_CURRENT_LIST_DIR}/include/
    )

  # Include subdirectory to build sub-component together as part of the kernel.
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/arch/armv7m)
elseif (ARCH_DEFINED STREQUAL "RV32I")
  target_compile_definitions(${LIBRARY_NAME} PRIVATE RV32I)

  target_include_directories(${LIBRARY_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/arch/rv32i/
    ${CMAKE_CURRENT_LIST_DIR}/include/
  )

  # Include subdirectory to build sub-component together as part of the kernel.
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/arch/rv32i)
else()
  message("'ARCH_DEFINED' is not recognized: ${ARCH_DEFINED}")
endif()



