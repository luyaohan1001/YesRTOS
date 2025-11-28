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

linkedlist<Thread>* PreemptFIFOScheduler::ready_list[MAX_PRIO_LEVEL];

list_node_t<Thread>* PreemptFIFOScheduler::running_threads[MAX_PRIO_LEVEL];

volatile uint32_t** PreemptFIFOScheduler::pp_active_thread_stk = nullptr;

size_t PreemptFIFOScheduler::curr_prio = 0;

bool PreemptFIFOScheduler::init_complete = false;

void PreemptFIFOScheduler::init() {
  mempool::init();

  for (size_t prio_idx = 0; prio_idx < MAX_PRIO_LEVEL; ++prio_idx) {
    linkedlist<Thread>* p_list = mempool::malloc_construct<linkedlist<Thread>>();
    // mempool::alloc_t alloc_res = mempool::malloc(sizeof(linkedlist<Thread>));
    // linkedlist<Thread>* p_list = reinterpret_cast<linkedlist<Thread>*>(alloc_res.addr);
    ready_list[prio_idx] = p_list;
    running_threads[prio_idx] = ready_list[prio_idx]->head;
  }

  PreemptFIFOScheduler::init_complete = true;
}

/**
 * @brief Add thread to scheduler queue.
 */
void PreemptFIFOScheduler::add_thread(Thread* t, size_t prio_level) {
  if (!init_complete) PreemptFIFOScheduler::init();

  linkedlist<Thread>* p_list = ready_list[prio_level];
  p_list->insert_front(*t);
}

/**
 * @brief start
 */
void PreemptFIFOScheduler::start() {
  if (!init_complete) PreemptFIFOScheduler::init();

  linkedlist<Thread>* p_list = nullptr;

  size_t prio_idx;
  for (prio_idx = 0; prio_idx < MAX_PRIO_LEVEL; ++prio_idx) {
    p_list = ready_list[prio_idx];
    if (p_list->head) break;
  }
  if (!p_list) return;

  PreemptFIFOScheduler::curr_prio = prio_idx;
  running_threads[prio_idx] = p_list->head;
  PreemptFIFOScheduler::pp_active_thread_stk = &p_list->head->data.stkptr;

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

  list_node_t<Thread>* p_curr_thread_node = running_threads[curr_prio];
  list_node_t<Thread>* p_next_thread_node = ready_list[curr_prio]->get_next_node_circular(p_curr_thread_node);

  // iterate the next ready list when tasks at current prio are not at ready state.
  if (p_next_thread_node == p_curr_thread_node && p_curr_thread_node->data.get_state() != ACTIVE ) {
    uint32_t prio_idx_prev = PreemptFIFOScheduler::curr_prio;
    uint32_t prio_idx;

    // iterate the next prio level ready list
    do {
      prio_idx = (PreemptFIFOScheduler::curr_prio + 1) % MAX_PRIO_LEVEL;
      if (ready_list[prio_idx] != nullptr) break;
    } while (prio_idx != prio_idx_prev);

    p_curr_thread_node = running_threads[prio_idx];
    p_next_thread_node = ready_list[prio_idx]->get_next_node_circular(p_curr_thread_node);
  }

  // TODO we need to implement an assert that explicitly prints error messages when it fails.
  // assert(p_next_thread_node);
  running_threads[curr_prio] = p_next_thread_node;

  PreemptFIFOScheduler::pp_active_thread_stk = &p_next_thread_node->data.stkptr;
}