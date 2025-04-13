# CMake version.
cmake_minimum_required(VERSION 3.10)

# Cross compilation tool chain.
include(${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/armv7m/gcc-arm-none-eabi.cmake)

# Linker script for address-related symbol resolution (stack start address, etc.)
set(LINKER_SCRIPT_PATH ${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/armv7m/STM32F767ZITx_FLASH.ld)
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T ${LINKER_SCRIPT_PATH}")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -T \"${LINKER_SCRIPT_PATH}\"")

set(PROJECT_NAME multi_thread)
project(${PROJECT_NAME} VERSION 1.0)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)

# Specify CMake subdirectory. It will parse CMakeLists.txt in specified directory, build and link with current project.
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel/build
)

# Header include directory.
target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel/include
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/armv7m
)

