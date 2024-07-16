/**
 * @file scheduler.hpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS scheduler header.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#pragma once
#include <armv7m.h>

#include <thread.hpp>

#define THREAD_QUEUE_DEPTH 2

namespace YesRTOS {
class RoundRobinScheduler {
 public:
  RoundRobinScheduler();
  ~RoundRobinScheduler();

  void add_thread(Thread* thread);
  void schedule();

 private:
  uint8_t q_top_cnt = 0;
  uint32_t sched_context_stack[CONTEXT_SAVE_STACK_SIZE];
  Thread* thread_q[THREAD_QUEUE_DEPTH];
};
}  // namespace YesRTOS
