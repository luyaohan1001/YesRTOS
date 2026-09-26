# CMake configuration for YesRTOS kernel build. Added from the top-level CMakeLists.txt; the toolchain comes from CMAKE_TOOLCHAIN_FILE.

# Specify C standard.
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Specify Cpp standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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
  add_compile_definitions(ARMV7M)

  target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/arch/armv7m/
    ${CMAKE_CURRENT_LIST_DIR}/include/
    )

  # Include subdirectory to build sub-component together as part of the kernel.
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/arch/armv7m)
else()
  message("'ARCH_DEFINED' is not recognized: ${ARCH_DEFINED}")
endif()



