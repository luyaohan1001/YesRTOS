# YesRTOS machine layer for ARMv7-M. Added from kernel/CMakeLists.txt; the toolchain comes from CMAKE_TOOLCHAIN_FILE.

# Add library which includes all other bare metal code.
add_library(LibBareMetal
    ${CMAKE_CURRENT_LIST_DIR}/startup_stm32f767xx.s
    ${CMAKE_CURRENT_LIST_DIR}/atomic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/timeslice.cpp
)

# QEMU does not emulate the ITM; route traces to USART1 instead.
if (QEMU)
  target_sources(LibBareMetal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/trace_qemu_usart.c)
  set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/qemu_netduinoplus2.ld)
else()
  target_sources(LibBareMetal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/trace.c)
  set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/STM32F767ZITx_FLASH.ld)
endif()

target_include_directories(LibBareMetal PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}   # ${CMAKE_CURRENT_LIST_DIR} maps to current folder: arch/armv7m
    ${CMAKE_CURRENT_LIST_DIR}/../../include
)

target_compile_definitions(LibBareMetal PRIVATE ARMV7M)

# Any executable linking the machine layer is linked with the board's memory layout, and relinked when it changes.
target_link_options(LibBareMetal INTERFACE -T ${LINKER_SCRIPT})
set_property(TARGET LibBareMetal APPEND PROPERTY INTERFACE_LINK_DEPENDS ${LINKER_SCRIPT})
