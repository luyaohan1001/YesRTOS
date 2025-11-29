/**
 * @file rr_scheduler.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS scheduler implementation.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include "preempt_fifo_scheduler.hpp"

#if defined(ARMV7M)
#include <baremetal_api.h>
#endif

#if defined(HOST_PLATFORM)
#include <iostream>
#endif

#include "mempool.hpp"

using namespace YesRTOS;

volatile uint32_t** PreemptFIFOScheduler::pp_active_thread_stk = nullptr;

size_t PreemptFIFOScheduler::curr_prio = 0;

bool PreemptFIFOScheduler::init_complete = false;

Thread* PreemptFIFOScheduler::ready_list = nullptr;
Thread* PreemptFIFOScheduler::current = nullptr;

void PreemptFIFOScheduler::init() {
  mempool::init();
  PreemptFIFOScheduler::init_complete = true;
}

/**
 * @brief Add thread to scheduler queue.
 */
void PreemptFIFOScheduler::add_thread(Thread* t, size_t prio_level) {
  if (!init_complete) PreemptFIFOScheduler::init();
  if (!ready_list) {
    ready_list = t;
  } else {
    t->thread_info.p_next = ready_list;
    ready_list = t;
  }
}

/**
 * @brief start
 */
void PreemptFIFOScheduler::start() {
  if (!init_complete) PreemptFIFOScheduler::init();
  current = ready_list;
  PreemptFIFOScheduler::pp_active_thread_stk = &current->stkptr;
#if defined (ARMV7M)
  itm_initialize();
  systick_clk_init();
  start_first_task();
#endif
}

/**
 * @brief Return the next thread to run.
 */
__attribute__((optimize("O0")))
void PreemptFIFOScheduler::schedule_next() {
  Thread* next_sched_thread = current->thread_info.p_next;
  if (!next_sched_thread) next_sched_thread = ready_list;
  current = next_sched_thread;
  PreemptFIFOScheduler::pp_active_thread_stk = &next_sched_thread->stkptr;
}
