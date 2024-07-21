/**
 * @file scheduler.hpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS scheduler header.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#pragma once
#include <thread.hpp>

#define THREAD_QUEUE_DEPTH 8

namespace YesRTOS {

class Scheduler {
  public:
  Scheduler() = default;
  ~Scheduler() = default;

  void add_thread(Thread* thread);
  Thread* thread_q[THREAD_QUEUE_DEPTH];

  protected:
  uint32_t q_size;
  uint32_t sched_context_stack[CONTEXT_SAVE_STACK_SIZE];
};

class PreemptiveScheduler : public Scheduler {
  public:
  PreemptiveScheduler();
  ~PreemptiveScheduler();

  void schedule_next();
  void start();

  /**
   * @brief Double pointer to stack of thread currently being executed.
   * @note This is a double pointer.
   *       *pp_active_thread_stk ==> the stack pointer, SP. Allocate space by (*pp_active_thread_stk)--, vise versa.
   *       **pp_active_thread_stk ==> content of the entry pointed by SP. We could write to that stack entry by (**pp_active_thread_stk)=value.
   */
  uint32_t** pp_active_thread_stk;

  private:
  uint32_t curr_thread_cnt;
};

}  // namespace YesRTOS
