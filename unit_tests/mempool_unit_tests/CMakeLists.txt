cmake_minimum_required(VERSION 3.10)
project(UnitTest)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Build Debug mode for gdb debugging
if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

# Include project source
# message(STATUS "LHDBG CMakeList.txt in unit_tests directory CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

# Enable compile command dumping to json
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(mempool_unit_test
  ${CMAKE_SOURCE_DIR}/mempool_unit_test.cpp
  ${CMAKE_SOURCE_DIR}/../../src/mempool.cpp
  ${CMAKE_SOURCE_DIR}/../../src/bitops.cpp
)

target_include_directories(mempool_unit_test PRIVATE
    ${CMAKE_SOURCE_DIR}/../../include)

enable_testing()
add_test(NAME mempool_unit_test0 COMMAND ./mempool_unit_test "alloc size < 32")
add_test(NAME mempool_unit_test1 COMMAND ./mempool_unit_test "alloc and free size < 32")
add_test(NAME mempool_unit_test2 COMMAND ./mempool_unit_test "alloc size 128")
add_test(NAME mempool_unit_test3 COMMAND ./mempool_unit_test "invalid size")