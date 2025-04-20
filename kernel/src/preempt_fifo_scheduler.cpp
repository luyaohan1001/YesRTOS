/**
 * @file rr_scheduler.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS scheduler implementation.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include "preempt_fifo_scheduler.hpp"

#include <baremetal_api.h>

#if defined(HOST_PLATFORM)
#include <iostream>
#endif

#include "mempool.hpp"

using namespace YesRTOS;

linkedlist<Thread>* PreemptFIFOScheduler::array_of_list[MAX_PRIO_LEVEL];

list_node_t<Thread>* PreemptFIFOScheduler::array_of_list_node[MAX_PRIO_LEVEL];

uint32_t** PreemptFIFOScheduler::pp_active_thread_stk = nullptr;

size_t PreemptFIFOScheduler::curr_prio = 0;

bool PreemptFIFOScheduler::init_complete = false;

void PreemptFIFOScheduler::init() {
  mempool::init();

  for (size_t prio_idx = 0; prio_idx < MAX_PRIO_LEVEL; ++prio_idx) {
    linkedlist<Thread>* p_list = mempool::malloc_construct<linkedlist<Thread>>();
    // mempool::alloc_t alloc_res = mempool::malloc(sizeof(linkedlist<Thread>));
    // linkedlist<Thread>* p_list = reinterpret_cast<linkedlist<Thread>*>(alloc_res.addr);
    array_of_list[prio_idx] = p_list;
    array_of_list_node[prio_idx] = array_of_list[prio_idx]->head;
  }

  PreemptFIFOScheduler::init_complete = true;
}

/**
 * @brief Add thread to scheduler queue.
 */
void PreemptFIFOScheduler::add_thread(Thread* t, size_t prio_level) {
  if (!init_complete) PreemptFIFOScheduler::init();

  linkedlist<Thread>* p_list = array_of_list[prio_level];
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
    p_list = array_of_list[prio_idx];
    if (p_list->head) break;
  }
  if (!p_list) return;

  PreemptFIFOScheduler::curr_prio = prio_idx;
  array_of_list_node[prio_idx] = p_list->head;
  PreemptFIFOScheduler::pp_active_thread_stk = &p_list->head->data.stkptr;

  itm_initialize();
  systick_clk_init();
  start_first_task();
}

/**
 * @brief Return the next thread to run.
 */
void PreemptFIFOScheduler::schedule_next() {
  list_node_t<Thread>* p_curr_thread_node = array_of_list_node[curr_prio];
  list_node_t<Thread>* p_next_thread_node = array_of_list[curr_prio]->get_next_node_circular(p_curr_thread_node);

  if (p_next_thread_node == p_curr_thread_node) {
    uint32_t prio_idx_prev = PreemptFIFOScheduler::curr_prio;
    uint32_t prio_idx;
    do {
      prio_idx = (PreemptFIFOScheduler::curr_prio + 1) % MAX_PRIO_LEVEL;
      if (array_of_list[prio_idx] != nullptr) break;
    } while (prio_idx != prio_idx_prev);

    p_curr_thread_node = array_of_list_node[prio_idx];
    p_next_thread_node = array_of_list[prio_idx]->get_next_node_circular(p_curr_thread_node);
  }

  array_of_list_node[curr_prio] = p_next_thread_node;

  PreemptFIFOScheduler::pp_active_thread_stk = &p_next_thread_node->data.stkptr;
}