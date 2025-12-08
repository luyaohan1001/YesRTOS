# CMake version.
cmake_minimum_required(VERSION 3.10)
project(UnitTest)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Build Debug mode for gdb debugging
if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

# Build flags
add_compile_definitions("-DHOST_PLATFORM")

add_executable(linkedlist_unit_test
  ${CMAKE_SOURCE_DIR}/linkedlist_unit_test.cpp
  ${CMAKE_SOURCE_DIR}/../../kernel/src/thread.cpp
  ${CMAKE_SOURCE_DIR}/../../kernel/src/mempool.cpp
  ${CMAKE_SOURCE_DIR}/../../kernel/src/linkedlist.cpp
)

target_include_directories(linkedlist_unit_test PRIVATE
    ${CMAKE_SOURCE_DIR}/../../kernel/include
    ${CMAKE_SOURCE_DIR}/../../kernel/arch/armv7m)

enable_testing()