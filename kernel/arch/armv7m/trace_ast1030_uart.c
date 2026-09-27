/**
 * @file trace_ast1030_uart.c
 * @brief Tracing backend for QEMU ast1030-evb, printing to UART5 in place of the ARM ITM.
 * @note QEMU does not emulate the ITM, so the itm_* API is kept and routed to UART5 instead, the 16550 compatible UART
 *       QEMU connects its first '-serial' device (stdio with -nographic) to. Registers are 4 bytes apart.
 * @version 0.1
 * @date 2026-09-26
 *
 * @copyright Copyright (c) 2026
 */

#include "stdint.h"

// Memory mapped registers of the AST1030 UART5 (16550 compatible, register shift 2).
#define UART5_THR              *((volatile uint32_t*) 0x7E784000U )  // Transmit Holding Register (DLAB = 0)
#define UART5_LCR              *((volatile uint32_t*) 0x7E78400CU )  // Line Control Register
#define UART5_LSR              *((volatile uint32_t*) 0x7E784014U )  // Line Status Register

#define UART_LCR_8N1           (0x03U)     // 8 data bits, no parity, 1 stop bit, DLAB = 0
#define UART_LSR_THRE          (1U << 5)   // Transmit holding register empty

/**
 * @brief Initialize UART5 for transmission (8N1; QEMU ignores the baud rate).
 */
void itm_initialize() {
  UART5_LCR = UART_LCR_8N1;
}

/**
 * @brief Transmit single character over UART5.
 * @param ch Character to transmit.
 */
void itm_write_char(char ch) {
  // block until the transmit holding register is empty.
  while (!(UART5_LSR & UART_LSR_THRE));

  UART5_THR = ch;
}

/**
 * @brief Trace string.
 * @param ptr Pointer to first character.
 */
void itm_trace(const char* ptr) {
  while (*ptr != '\0') {
    itm_write_char(*ptr++);
  }
}
