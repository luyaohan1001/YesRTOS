# YesRTOS machine layer for ARMv7-M. Added from kernel/CMakeLists.txt; the toolchain comes from CMAKE_TOOLCHAIN_FILE.

# Add library which includes all other bare metal code.
add_library(LibBareMetal
    ${CMAKE_CURRENT_LIST_DIR}/startup_stm32f767xx.s
    ${CMAKE_CURRENT_LIST_DIR}/atomic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/timeslice.cpp
)

# Per board: trace output, memory layout, the core clock SysTick counts (CPU_CLK_FREQ_HZ, default in config.h) and the
# number of external interrupt vectors in startup_stm32f767xx.s (YESRTOS_NUM_IRQS).
if (BOARD STREQUAL "netduinoplus2")
  # QEMU does not emulate the ITM; route traces to USART1 instead.
  target_sources(LibBareMetal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/trace_qemu_usart.c)
  set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/qemu_netduinoplus2.ld)
  target_compile_definitions(LibBareMetal PUBLIC CPU_CLK_FREQ_HZ=168000000UL)   # QEMU runs the STM32F405 at 168 MHz
  target_compile_definitions(LibBareMetal PRIVATE YESRTOS_NUM_IRQS=96)          # IRQs QEMU delivers (0..95, measured)
elseif (BOARD STREQUAL "ast1030")
  # Aspeed AST1030 (QEMU ast1030-evb): traces on the 16550 UART5, program and data in its 768 KB SRAM.
  target_sources(LibBareMetal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/trace_ast1030_uart.c)
  set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/qemu_ast1030.ld)
  target_compile_definitions(LibBareMetal PUBLIC CPU_CLK_FREQ_HZ=200000000UL)   # QEMU runs the AST1030 at 200 MHz
  target_compile_definitions(LibBareMetal PRIVATE YESRTOS_NUM_IRQS=272)         # external interrupts of QEMU's NVIC
else()
  target_sources(LibBareMetal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/trace.c)
  set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/STM32F767ZITx_FLASH.ld)
  target_compile_definitions(LibBareMetal PRIVATE YESRTOS_NUM_IRQS=110)         # STM32F76x external interrupts
endif()

target_include_directories(LibBareMetal PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}   # ${CMAKE_CURRENT_LIST_DIR} maps to current folder: arch/armv7m
    ${CMAKE_CURRENT_LIST_DIR}/../../include
)

target_compile_definitions(LibBareMetal PRIVATE ARMV7M)

# Any executable linking the machine layer is linked with the board's memory layout, and relinked when it changes.
target_link_options(LibBareMetal INTERFACE -T ${LINKER_SCRIPT})
set_property(TARGET LibBareMetal APPEND PROPERTY INTERFACE_LINK_DEPENDS ${LINKER_SCRIPT})
