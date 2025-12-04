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

Thread* PreemptFIFOScheduler::p_active_thread = nullptr;

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
void PreemptFIFOScheduler::add_thread(Thread* p_new, size_t prio_level) {
  if (!init_complete) PreemptFIFOScheduler::init();
  if (!ready_list) {
    // head [p_new] ==> nullptr
    ready_list = p_new;
    p_new->thread_info.p_next = nullptr;
    p_new->thread_info.p_prev = nullptr;
  } else {
    // head [p_new] <==> existing node <==> existing node ==> nullptr
    ready_list->thread_info.p_prev = p_new;
    p_new->thread_info.p_next = ready_list;
    p_new->thread_info.p_prev = nullptr;
    ready_list = p_new;
  }
}

/**
 * @brief start
 */
void PreemptFIFOScheduler::start() {
  if (!init_complete) PreemptFIFOScheduler::init();
  current = ready_list;
  PreemptFIFOScheduler::p_active_thread = current;
#if defined (ARMV7M)
  itm_initialize();
  systick_clk_init();
  start_first_task();
#endif
}

/**
 * @brief Return the next thread to run.
 */
void PreemptFIFOScheduler::schedule_next() {
  Thread* next_sched_thread = current->thread_info.p_next;
  if (!next_sched_thread) next_sched_thread = ready_list;
  current = next_sched_thread;
  PreemptFIFOScheduler::p_active_thread = next_sched_thread;
}


void PreemptFIFOScheduler::move_node(Thread** src_list, Thread** dest_list, Thread* node) {
    if (!node) {
        return;
    }

    // node deletion
    Thread* p_prev = node->thread_info.p_prev;
    Thread* p_next = node->thread_info.p_next;
    if (p_prev) {
        // prev ==> p_next
        p_prev->thread_info.p_next = node->thread_info.p_next;
    } else {
        // (head) p_next 
        *src_list = node->thread_info.p_next;
    }
    if (p_next) p_next->thread_info.p_prev = p_prev;

    // node addition to the new list
    if (*dest_list) {
        (*dest_list)->thread_info.p_prev = node;
        node->thread_info.p_next = *dest_list;
        node->thread_info.p_prev = nullptr;
        *dest_list = node;
    } else {
        node->thread_info.p_next = nullptr;
        node->thread_info.p_prev = nullptr;
        *dest_list = node;
    }
}
