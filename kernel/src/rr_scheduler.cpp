/**
 * @file rr_scheduler.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS scheduler implementation.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include <baremetal_api.h>

#include <mempool.hpp>
#include <rr_scheduler.hpp>

#if defined(HOST_PLATFORM)
#include <iostream>
#endif

using namespace YesRTOS;

// Static storage definition with internal linkage. https://en.cppreference.com/w/c/language/static_storage_duration
Thread* RoundRobinScheduler::thread_q[TASK_QUEUE_DEPTH];

volatile uint32_t** RoundRobinScheduler::pp_active_thread_stk = nullptr;

uint32_t RoundRobinScheduler::curr_thread_cnt;

uint32_t RoundRobinScheduler::q_size = 0;

/**
 * @brief Add thread to scheduler queue.
 */
void RoundRobinScheduler::add_thread(Thread* t) {
  RoundRobinScheduler::thread_q[q_size++] = t;
}

/**
 * @brief start
 */
void RoundRobinScheduler::start() {
  if (RoundRobinScheduler::q_size == 0) {
    return;
  }

  RoundRobinScheduler::curr_thread_cnt = 0;
  RoundRobinScheduler::pp_active_thread_stk = &RoundRobinScheduler::thread_q[RoundRobinScheduler::curr_thread_cnt]->stkptr;
  systick_clk_init();
  start_first_task();
}

/**
 * @brief Return the next thread to run.
 */
void RoundRobinScheduler::schedule_next() {
  RoundRobinScheduler::curr_thread_cnt += 1;
  RoundRobinScheduler::curr_thread_cnt %= RoundRobinScheduler::q_size;
  Thread* thread_ptr = RoundRobinScheduler::thread_q[RoundRobinScheduler::curr_thread_cnt];
  RoundRobinScheduler::pp_active_thread_stk = &thread_ptr->stkptr;
}
