/**
 * @file armv7m.h
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Machine layer interface specific to ARM-V7 Cortex-M Architecture.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */
#pragma once

#define CONTEXT_SAVE_STACK_SIZE 9

#include <cstdint>

/**
 * @brief Interfaces implemented in context.s.
 */
extern "C" {
  extern void yield_thread(void *thread_handle);
  extern void save_sched_context();
}

/**
 * @brief Interfaces implemented in trace.c
 *
 */
extern "C" {
  extern void itm_trace(const char* ptr);
}