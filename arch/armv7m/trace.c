/**
 * @file trace.c
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Tracing library based on the ARM Instrumentation Trace Macrocell.
 * @version 0.1
 * @date 2024-07-15
 *
 * @copyright Copyright (c) 2024
 */

#include "stdint.h"

// Memory mapped addresses for ARM-v7 M core registers.

#define DEMCR                  *((volatile uint32_t*) 0xE000EDFCU ) // Debug Exception and Monitor Control Register
#define ITM_STIM0              *((volatile uint32_t*) 0xE0000000 )  // Stimulus Port Register 0
#define ITM_TER0               *((volatile uint32_t*) 0xE0000E00 )  // Trace Enable Register 0

/**
 * @brief Initialize instrumentation trace macrocell.
 */
void itm_initialize() {
  DEMCR |= (1 << 24);
  ITM_TER0 |= (1 << 0);
}

/**
 * @brief Push to instrumentation trace macrocell fifo.
 *
 * @param ch Character to push to fifo.
 */
void itm_write_char(char ch) {
  // block FIFO until it's clear
  while(!(ITM_STIM0 & 1));

  // Write to ITM stimulus port0
  ITM_STIM0 = ch;
}

/**
 * @brief Trace string.
 * @param ptr Pointer to first character.
 * @todo Add lock to function for thread safety.
 */
void itm_trace(const char* ptr) {
  itm_initialize();
  do {
    itm_write_char(*ptr++);
  } while (*ptr != '\0');
}