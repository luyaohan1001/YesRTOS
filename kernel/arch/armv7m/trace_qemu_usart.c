/**
 * @file trace_qemu_usart.c
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Tracing backend for QEMU netduinoplus2, printing to USART1 in place of the ARM ITM.
 * @note QEMU does not emulate the ITM, so the itm_* API is kept and routed to USART1 instead.
 *       QEMU connects its first '-serial' device (stdio with -nographic) to USART1.
 * @version 0.1
 * @date 2026-09-24
 *
 * @copyright Copyright (c) 2026
 */

#include "stdint.h"

// Memory mapped addresses for STM32F405 USART1.
#define USART1_SR              *((volatile uint32_t*) 0x40011000U )  // Status Register
#define USART1_DR              *((volatile uint32_t*) 0x40011004U )  // Data Register
#define USART1_CR1             *((volatile uint32_t*) 0x4001100CU )  // Control Register 1

#define USART_SR_TXE           (1U << 7)   // Transmit data register empty
#define USART_CR1_UE           (1U << 13)  // USART enable
#define USART_CR1_TE           (1U << 3)   // Transmitter enable

/**
 * @brief Initialize USART1 for transmission.
 */
void itm_initialize() {
  USART1_CR1 = USART_CR1_UE | USART_CR1_TE;
}

/**
 * @brief Transmit single character over USART1.
 * @param ch Character to transmit.
 */
void itm_write_char(char ch) {
  // block until data register is empty.
  while (!(USART1_SR & USART_SR_TXE));

  USART1_DR = ch;
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
