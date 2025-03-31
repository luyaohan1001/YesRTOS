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
#include <scheduler.hpp>

using namespace YesRTOS;

/**
 * @brief Add thread to scheduler queue.
 */
void Scheduler::add_thread(Thread *t) {
  this->thread_q[q_size++] = t;
}

/**
 * @brief Constructor
 */
PreemptiveScheduler::PreemptiveScheduler() {
  this->q_size = 0;
  this->pp_active_thread_stk = nullptr;
}

/**
 * @brief Destructor
 */
PreemptiveScheduler::~PreemptiveScheduler() {
  return;
}

/**
 * @brief Destructor
 */
void PreemptiveScheduler::start() {
  if (q_size == 0) {
    return;
  }

  this->curr_thread_cnt = 0;
  this->pp_active_thread_stk = &thread_q[this->curr_thread_cnt]->stkptr;
  systick_clk_init();
  start_first_task();
}

/**
 * @brief Return the next thread to run.
 */
void PreemptiveScheduler::schedule_next() {
  // RoundRobin Scheduling.
  // TODO: Add logic for thread priority based scheduling.
  this->curr_thread_cnt += 1;
  this->curr_thread_cnt %= this->q_size;
  Thread *thread_ptr = this->thread_q[this->curr_thread_cnt];
  this->pp_active_thread_stk = &thread_ptr->stkptr;
}