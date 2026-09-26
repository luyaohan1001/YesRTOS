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

/**
 * @brief Called by the idle thread on every pass of its loop, before it waits for the next interrupt.
 * @note  Weak default does nothing; an application may define it (e.g. for power management or statistics). It runs in
 *        the idle thread, so it must never block.
 */
extern "C" void yesrtos_idle_hook(void);

namespace YesRTOS {

class PreemptFIFOScheduler final {
  public:
  static void schedule_next();
  static void start();
  static void init();

  /**
   * @brief Make a thread ready. Safe to call before start() and from a running thread.
   * @param thread Thread to add; its priority must be below MAX_PRIO_LEVEL (that level is reserved for the idle thread).
   * @return false, with nothing changed, for a null thread or a priority out of range.
   */
  static bool add_thread(Thread* thread);

  /**
   * @brief Add running thread to a blocked list and marked as BLOCKED.
   * @param pp_blocked_list_head Double pointer to the head of a blocked list maintain by other entity such as a YesRTOS::Mutex.
   */
  static void block_running_thread(Thread** pp_blocked_list_head);
  /**
   * @brief Move the longest waiting thread from blocked list to ready list and marked as READY.
   * @param pp_blocked_list_head Double pointer to the head of a blocked list maintain by other entity such as a YesRTOS::Mutex.
   * @return Pointer to the unblocked thread, or nullptr if the blocked list is empty.
   */
  static Thread* unblock_one_thread(Thread** pp_blocked_list_head);

  /**
   * @brief End the running thread: move it to the completed list, mark it COMPLETE and switch away for good.
   * @note  Entered when a thread routine returns (its initial LR is yesrtos_thread_exit()). Never returns.
   */
  [[noreturn]] static void exit_running_thread();

  /**
   * @brief Threads whose routine returned, most recent first.
   */
  static Thread* completed_list;

  /**
   * @brief Pointer to the thread currently being executed.
   * @note This points to the active Thread object.
   *       - p_active_thread       ==> pointer to the currently running thread.
   *       - *p_active_thread      ==> the Thread object itself.
   *       This is used by the scheduler and context switching routines to track and manipulate the currently active thread.
   */
  static Thread* p_active_thread;

  static bool init_complete;

  /**
   * @brief Priority level of the idle thread: one below the lowest user priority (MAX_PRIO_LEVEL - 1), so it only runs
   *        when no user thread is ready.
   */
  static constexpr uint8_t IDLE_PRIO = MAX_PRIO_LEVEL;

  /**
   * @brief Ready list per priority level; the extra last level holds only the idle thread.
   */
  static Thread* ready_list_heads[MAX_PRIO_LEVEL + 1];

  /**
   * @brief Bitmap encoding non-empty ready list of a specific priority.
   */
  static uint32_t prio_bitmap;

  private:
  /**
   * @note Static class. Hide constructor / destructor to avoid instantiation.
   */

  static void move_node(Thread** src_list, Thread** dest_list, Thread* node);

  static void insert_ready(Thread* p_new);

  PreemptFIFOScheduler() = delete;
  ~PreemptFIFOScheduler() = delete;

};

}  // namespace YesRTOS
