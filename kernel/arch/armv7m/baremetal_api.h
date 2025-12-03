/**
 * @file baremetal_api.h
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Bare metal API provided in C.
 * @version 1.0
 * @date 2024-07-16
 * @copyright Copyright (c) 2024
 */

#include "stdint.h"
#include "stdbool.h"

/**
 * @brief APIs
 */
#if defined(__cplusplus)
extern "C" {
#endif
  //  Interfaces implemented in trace.c
  extern void itm_initialize();
  extern void itm_trace(const char* ptr);
  // Interfaces implemented in timeslice.c
  extern void __attribute__((naked)) PendSV_Handler();
  extern void request_context_switch();
  extern void start_first_task();
  extern void systick_clk_init(void);
  extern void init_stack_armv7m(volatile uint32_t** pp_stk, uint32_t* routine_ptr);
  extern bool atomic_compare_and_swap(volatile uint32_t *p_mem, uint32_t old_val, uint32_t new_val);
  void disable_exception();
  void enable_exception();
#if defined(__cplusplus)
}
#endif
