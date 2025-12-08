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

  static void add_thread(Thread* thread);

  /**
   * @brief Add running thread to a blocked list and marked as BLOCKED.
   * @param pp_blocked_list_head Double pointer to the head of a blocked list maintain by other entity such as a YesRTOS::Mutex.
   */
  static void block_running_thread(Thread** pp_blocked_list_head);
  /**
   * @brief Move another blocked thread from blocked list to ready list and marked as READY.
   * @param pp_blocked_list_head Double pointer to the head of a blocked list maintain by other entity such as a YesRTOS::Mutex.
   */
  static void unblock_one_thread(Thread** pp_blocked_list_head);

  /**
   * @brief Pointer to the thread currently being executed.
   * @note This points to the active Thread object.
   *       - p_active_thread       ==> pointer to the currently running thread.
   *       - *p_active_thread      ==> the Thread object itself.
   *       This is used by the scheduler and context switching routines to track and manipulate the currently active thread.
   */
  static Thread* p_active_thread;

  static bool init_complete;

  static Thread* ready_list_heads[MAX_PRIO_LEVEL];

  /**
   * @brief Bitmap encoding non-empty ready list of a specific priority.
   */
  static uint32_t prio_bitmap;

  private:
  /**
   * @note Static class. Hide constructor / destructor to avoid instantiation.
   */

  static void move_node(Thread** src_list, Thread** dest_list, Thread* node);

  PreemptFIFOScheduler() = delete;
  ~PreemptFIFOScheduler() = delete;

};

}  // namespace YesRTOS
