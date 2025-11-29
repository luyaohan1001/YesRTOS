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
   * @brief Pointer to the thread currently being executed.
   * @note This points to the active Thread object.
   *       - p_active_thread       ==> pointer to the currently running thread.
   *       - *p_active_thread      ==> the Thread object itself.
   *       This is used by the scheduler and context switching routines
   *       to track and manipulate the currently active thread.
   */
  volatile static Thread* p_active_thread;


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
