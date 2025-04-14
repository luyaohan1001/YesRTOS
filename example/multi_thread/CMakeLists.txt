# CMake version.
cmake_minimum_required(VERSION 3.10)

# Cross compilation tool chain.
include(${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/armv7m/gcc-arm-none-eabi.cmake)

# Linker script for address-related symbol resolution (stack start address, etc.)
set(LINKER_SCRIPT_PATH ${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/armv7m/STM32F767ZITx_FLASH.ld)
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T ${LINKER_SCRIPT_PATH}")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -T \"${LINKER_SCRIPT_PATH}\"")

# Build Debug mode for gdb debugging.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(PROJECT_NAME multi_thread)
project(${PROJECT_NAME} VERSION 1.0)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)

# Specify CMake subdirectory. It will parse CMakeLists.txt in specified directory, and build artifact for that CMakeList.txt.
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel/build
)

# Header include directory.
target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel/include
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/armv7m
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  LibYesRTOSKernel
  LibStartupASM
  LibBareMetal
)

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
        --size-sort
        --print-size
        --numeric-sort
        --line-numbers
        --radix=x
        multi_thread.elf > symtable.dump
)

# Dump size from ELF (executable and linkable format) file.
add_custom_target(dump_section_size
        arm-none-eabi-size -A
        multi_thread.elf > size.dump
)

