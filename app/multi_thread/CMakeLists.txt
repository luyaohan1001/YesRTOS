# CMake version.
cmake_minimum_required(VERSION 3.10)

# Linker script for address-related symbol resolution (stack start address, etc.)
if (ARCH_DEFINED STREQUAL "ARMV7M")
  include(${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/armv7m/gcc-arm-none-eabi.cmake)
elseif (ARCH_DEFINED STREQUAL "RV32I")
  include(${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/rv32i/gcc-riscv-none-eabi.cmake)
else()
  message("'ARCH_DEFINED' is not recognized: ${ARCH_DEFINED}")
endif()

# set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -T \"${LINKER_SCRIPT_PATH}\"")

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

if (ARCH_DEFINED STREQUAL "ARMV7M")
  target_compile_definitions(${PROJECT_NAME} PRIVATE ARMV7M)
elseif (ARCH_DEFINED STREQUAL "RV32I")
  target_compile_definitions(${PROJECT_NAME} PRIVATE RV32I)
else()
  message("'ARCH_DEFINED' is not recognized: ${ARCH_DEFINED}")
endif()

# Enable compile command export to json (for CMake debugging).
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)


# Specify CMake subdirectory. It will parse CMakeLists.txt in specified directory, and build artifact for that CMakeList.txt.
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel/build
)

# Header include directory.
target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel/include
  /Users/lhan/Projects/YesRTOS/compiler/arm-gnu-toolchain-14.3.rel1-darwin-arm64-arm-none-eabi/arm-none-eabi/include/
)

if (ARCH_DEFINED STREQUAL "ARMV7M")
  target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/armv7m
  )
elseif (ARCH_DEFINED STREQUAL "RV32I")
  target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/rv32i
  )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
  LibYesRTOSKernel
  LibBareMetal
)

# Kill debug session
add_custom_target(kill
COMMAND pkill openocd
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

