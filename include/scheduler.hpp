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

#include "config.h"

namespace YesRTOS {

/*
@note
  With Templates: When templates are used, the compiler only knows the types when it instantiates the class with specific types.
  Because of this, the compiler needs explicit instructions to access members from the base class.
  Thus 'this->' is required to explicitly access members inherited from a templated base class.
*/
template <uint32_t DEPTH>
class RoundRobinScheduler {
  public:
  static void schedule_next();
  static void start();

  static void add_thread(Thread* thread);
  static Thread* thread_q[DEPTH];

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

  protected:
  static uint32_t q_size;
  static uint32_t sched_context_stack[CONTEXT_SAVE_STACK_SIZE];
};

}  // namespace YesRTOS
