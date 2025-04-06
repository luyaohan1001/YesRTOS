/**
 * @file preempt_fifo_scheduler.hpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Class definition for a preemptive scheduler. The classes are designed for singleton pattern.
 * @note Defining class as singleton pattern requires 'static' on each method, which avoid global instance. However, it limits polymorphism and flexibility.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#pragma once
#include <thread.hpp>

#include "config.h"
#include "linkedlist.hpp"
#include "mempool.hpp"

namespace YesRTOS {

class PreemptFIFOScheduler final {
  public:
  static void schedule_next();
  static void start();
  static void init();

  static void add_thread(Thread* thread, size_t prio_level);

  /**
   * @brief Double pointer to stack of thread currently being executed.
   * @note This is a pointer to pointer.
   *       *pp_active_thread_stk ==> the stack pointer, SP. Allocate space by (*pp_active_thread_stk)--, vise versa.
   *       **pp_active_thread_stk ==> content of the entry pointed by SP. We could write to that stack entry by (**pp_active_thread_stk)=value.
   */
  static uint32_t** pp_active_thread_stk;

  static size_t curr_prio;  // records priority index of list to visit to schedule next thread.

  static bool init_complete;

  private:
  /**
   * @note Static class. Hide constructor / destructor to avoid instantiation.
   */
  PreemptFIFOScheduler() = delete;
  ~PreemptFIFOScheduler() = delete;

  private:
  static linkedlist<Thread>* array_of_list[MAX_PRIO_LEVEL];        // points to list of threads for each priority level.
  static list_node_t<Thread>* array_of_list_node[MAX_PRIO_LEVEL];  // points to node of execution for each priority level.
};

}  // namespace YesRTOS