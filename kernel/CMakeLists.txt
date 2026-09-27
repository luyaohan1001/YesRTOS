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
  ${CMAKE_CURRENT_LIST_DIR}/src/linkedlist.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/spinlock.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/mutex.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/semaphore.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/heap.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/o1heap/o1heap.c
)

# Vendored o1heap allocator behind YesRTOS::Heap; its build options live in src/o1heap_config.h.
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../third_party/o1heap)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/../third_party/o1heap/o1heap.c PROPERTIES
  COMPILE_DEFINITIONS "O1HEAP_CONFIG_HEADER=\"${CMAKE_CURRENT_LIST_DIR}/src/o1heap_config.h\"")

if (ARCH_DEFINED STREQUAL "ARMV7M")
  add_compile_definitions(ARMV7M)

  target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/arch/armv7m/
    ${CMAKE_CURRENT_LIST_DIR}/include/
    )

  # Include subdirectory to build sub-component together as part of the kernel.
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/arch/armv7m)

  # The kernel and the machine layer call into each other (e.g. scheduler -> request_context_switch(), PendSV ->
  # scheduler). Declaring the cycle makes CMake repeat both archives on the link line, so an image that never calls
  # the scheduler itself still resolves the machine layer's references into the kernel.
  target_link_libraries(${LIBRARY_NAME} PUBLIC LibBareMetal)
  target_link_libraries(LibBareMetal PUBLIC ${LIBRARY_NAME})
else()
  message("'ARCH_DEFINED' is not recognized: ${ARCH_DEFINED}")
endif()



