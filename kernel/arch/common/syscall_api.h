/**
 * @file syscall_api.h
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Provides APIs for syscall supporting operations from kernel such as software interrupt or context switch.
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
  // Interfaces implemented in context.s
  extern void yield_thread(void *thread_handle);
  extern void save_sched_context();
  extern void start_first_task();
  //  Interfaces implemented in trace.c
  extern void itm_initialize();
  extern void itm_trace(const char* ptr);
  // Interfaces implemented in timeslice.c
  extern void systick_clk_init(void);
  extern void init_stack_armv7m(uint32_t** pp_stk, uint32_t* routine_ptr);
  void disable_exception();
  void enable_exception();
#if defined(__cplusplus)
}
#endif
