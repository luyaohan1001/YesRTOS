/**
 * @file scheduler.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS scheduler implementation.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include <iostream>
#include <scheduler.hpp>

using namespace YesRTOS;

/**
 * @brief Pointer to scheduler context-saving stack.
 */
volatile uint32_t *sched_stk_ptr;

/**
 * @brief Constructor.
 */
RoundRobinScheduler::RoundRobinScheduler() {
  // save to global symbol
  sched_stk_ptr = this->sched_context_stack;
}

/**
 * @brief Destructor.
 */
RoundRobinScheduler::~RoundRobinScheduler() {
}

/**
 * @brief Pick the next thread and run.
 */
void RoundRobinScheduler::schedule() {
  while (true) {
    // Save scheduler context so when switching back the next task will be selected.
    save_sched_context();
    for (auto q_idx = 0; q_idx < THREAD_QUEUE_DEPTH; ++q_idx) {
      Thread *thread_ptr = thread_q[q_idx];
      switch (thread_ptr->get_state()) {
        case ACTIVE:
          thread_ptr->run();
          break;

        case RUNNING:
          break;

        case BLOCKED:
          break;

        case SLEEP:
          break;

        case COMPLETE:
          // TODO: Implement task deletion from thread queue.
          break;
      }
    }
  }
}

/**
 * @brief Add thread to scheduler queue.
 */
void RoundRobinScheduler::add_thread(Thread *t) {
  this->thread_q[q_top_cnt++] = t;
}
