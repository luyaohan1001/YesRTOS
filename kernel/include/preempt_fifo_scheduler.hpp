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

  static void move_node(Thread** src_list, Thread** dest_list, Thread* node);

  /**
   * @brief Pointer to the thread currently being executed.
   * @note This points to the active Thread object.
   *       - p_active_thread       ==> pointer to the currently running thread.
   *       - *p_active_thread      ==> the Thread object itself.
   *       This is used by the scheduler and context switching routines to track and manipulate the currently active thread.
   */
  static Thread* p_active_thread;

  static size_t curr_prio;  // records priority index of list to visit to schedule next thread.

  static bool init_complete;

  static Thread* ready_list;

  static Thread* current;
  private:
  /**
   * @note Static class. Hide constructor / destructor to avoid instantiation.
   */
  PreemptFIFOScheduler() = delete;
  ~PreemptFIFOScheduler() = delete;

  // static linkedlist<Thread>* ready_list[MAX_PRIO_LEVEL];        // points to list of threads for each priority level.
  // static list_node_t<Thread>* running_threads[MAX_PRIO_LEVEL];  // points to node of execution for each priority level.
};

}  // namespace YesRTOS
