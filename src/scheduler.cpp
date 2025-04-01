/**
 * @file scheduler.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS scheduler implementation.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include <baremetal_api.h>

#include <iostream>
#include <mempool.hpp>
#include <scheduler.hpp>

using namespace YesRTOS;

template <uint32_t DEPTH>
Thread* RoundRobinScheduler<DEPTH>::thread_q[DEPTH];

template <uint32_t DEPTH>
uint32_t** RoundRobinScheduler<DEPTH>::pp_active_thread_stk = nullptr;

template <uint32_t DEPTH>
uint32_t RoundRobinScheduler<DEPTH>::curr_thread_cnt;

template <uint32_t DEPTH>
uint32_t RoundRobinScheduler<DEPTH>::q_size = 0;

template <uint32_t DEPTH>
uint32_t RoundRobinScheduler<DEPTH>::sched_context_stack[CONTEXT_SAVE_STACK_SIZE];

/**
 * @brief Add thread to scheduler queue.
 */
template <uint32_t DEPTH>
void RoundRobinScheduler<DEPTH>::add_thread(Thread* t) {
  RoundRobinScheduler<DEPTH>::thread_q[q_size++] = t;
}

/**
 * @brief start
 */
template <uint32_t DEPTH>
void RoundRobinScheduler<DEPTH>::start() {
  if (RoundRobinScheduler<DEPTH>::q_size == 0) {
    return;
  }

  RoundRobinScheduler<DEPTH>::curr_thread_cnt = 0;
  RoundRobinScheduler<DEPTH>::pp_active_thread_stk = &RoundRobinScheduler<DEPTH>::thread_q[RoundRobinScheduler<DEPTH>::curr_thread_cnt]->stkptr;
  systick_clk_init();
  start_first_task();
}

/**
 * @brief Return the next thread to run.
 */
template <uint32_t DEPTH>
void RoundRobinScheduler<DEPTH>::schedule_next() {
  // RoundRobin Scheduling.
  // TODO: Add logic for thread priority based scheduling.
  RoundRobinScheduler<DEPTH>::curr_thread_cnt += 1;
  RoundRobinScheduler<DEPTH>::curr_thread_cnt %= RoundRobinScheduler<DEPTH>::q_size;
  Thread* thread_ptr = RoundRobinScheduler<DEPTH>::thread_q[RoundRobinScheduler<DEPTH>::curr_thread_cnt];
  RoundRobinScheduler<DEPTH>::pp_active_thread_stk = &thread_ptr->stkptr;
}