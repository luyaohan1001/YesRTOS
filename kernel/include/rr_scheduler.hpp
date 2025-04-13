/**
 * @file rr_scheduler.hpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS scheduler definition for round-robin scheduler. The classes are designed for singleton pattern.
 * @note Defining class as singleton pattern requires 'static' on each method, which avoid global instance. However, it limits polymorphism and flexibility.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#pragma once
#include <thread.hpp>

#include "config.h"

namespace YesRTOS {

class RoundRobinScheduler final {
  public:
  static void schedule_next();
  static void start();

  static void add_thread(Thread* thread);
  static Thread* thread_q[TASK_QUEUE_DEPTH];

  /**
   * @brief Double pointer to stack of thread currently being executed.
   * @note This is a double pointer.
   *       *pp_active_thread_stk ==> the stack pointer, SP. Allocate space by (*pp_active_thread_stk)--, vise versa.
   *       **pp_active_thread_stk ==> content of the entry pointed by SP. We could write to that stack entry by (**pp_active_thread_stk)=value.
   */
  static uint32_t** pp_active_thread_stk;

  private:
  static uint32_t curr_thread_cnt;
  /**
   * @note Hide constructor / destructor to avoid instantiation.
   */
  RoundRobinScheduler();
  ~RoundRobinScheduler();

  static uint32_t q_size;
};

}  // namespace YesRTOS
