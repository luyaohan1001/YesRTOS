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
#include "baremetal_api.h"
#endif

#if defined(HOST_PLATFORM)
#include <iostream>
#endif

#include "bitops.hpp"

using namespace YesRTOS;

Thread* PreemptFIFOScheduler::p_active_thread = nullptr;

bool PreemptFIFOScheduler::init_complete = false;

Thread* PreemptFIFOScheduler::ready_list_heads[MAX_PRIO_LEVEL] {nullptr};

uint32_t PreemptFIFOScheduler::prio_bitmap;

void PreemptFIFOScheduler::init() {
  PreemptFIFOScheduler::init_complete = true;
}

/**
 * @brief Add thread to scheduler queue.
 */
void PreemptFIFOScheduler::add_thread(Thread* p_new) {
  if (!init_complete) PreemptFIFOScheduler::init();

  uint8_t prio_level = p_new->thread_info.priority;

  Thread** pp_head = &ready_list_heads[prio_level];

  if (!(*pp_head)) {
    // create new list
    *pp_head = p_new;
    p_new->thread_info.p_next = nullptr;
    p_new->thread_info.p_prev = nullptr;

    set_bitpos<uint32_t>(prio_bitmap, prio_level);
  } else {
    // insert to head
    (*pp_head)->thread_info.p_prev = p_new;
    p_new->thread_info.p_next = *pp_head;
    p_new->thread_info.p_prev = nullptr;
    *pp_head = p_new;
  }
}

/**
 * @brief Start preemptive scheduler.
 */
void PreemptFIFOScheduler::start() {
  if (!init_complete) PreemptFIFOScheduler::init();

  uint32_t prio = count_trailing_zero<uint32_t>(prio_bitmap);
  PreemptFIFOScheduler::p_active_thread = ready_list_heads[prio];

#if defined (ARMV7M)
  itm_initialize();
  systick_clk_init();
  start_first_task();
#else
  #error "Timeslice not supported for undefined architecture."
#endif
}

static Thread* get_next_thread_circular(Thread *p_thread, Thread *p_head) {
  Thread *p_next = p_thread->thread_info.p_next;
  // Circular wrap around.
  if (!p_next) {
    p_next = p_head;
  }
  return p_next;
}

/**
 * @brief Return the next thread to run.
 */
void PreemptFIFOScheduler::schedule_next() {
  // Find highest priority ready task.
  Thread *p_next_ready;
  uint32_t prio = count_trailing_zero<uint32_t>(prio_bitmap);
  if (prio == p_active_thread->thread_info.priority) {
    p_next_ready = get_next_thread_circular(p_active_thread, ready_list_heads[prio]);
  } else {
    p_next_ready = ready_list_heads[prio];
  }
  PreemptFIFOScheduler::p_active_thread = p_next_ready;
}

void PreemptFIFOScheduler::move_node(Thread** src_list, Thread** dest_list, Thread* node) {
    if (!node) {
        return;
    }

    /*
    Deletion from source list:
    If previous node exists, link previous to next. Else, mark next as the new node.
    If next node exits, link its prev pointer to previous.
    */
    Thread* p_prev = node->thread_info.p_prev;
    Thread* p_next = node->thread_info.p_next;
    if (p_prev) {
        p_prev->thread_info.p_next = node->thread_info.p_next;
    } else {
        *src_list = node->thread_info.p_next;
    }
    if (p_next) p_next->thread_info.p_prev = p_prev;

    /*
    Addition to the destination list head.
    If list exists, make the node its new head, linking the node to its original head.
    If list does not exist, create the list by pointing the head to this node.
    */
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

void PreemptFIFOScheduler::block_running_thread(Thread** pp_blocked_list_head) {
  Thread *p_thread = p_active_thread;
  PreemptFIFOScheduler::move_node(&ready_list_heads[p_thread->thread_info.priority], pp_blocked_list_head, p_thread);
  p_thread->thread_info.state = BLOCKED;

  if (ready_list_heads[p_thread->thread_info.priority] == nullptr) {
    clr_bitpos<uint32_t>(prio_bitmap, p_thread->thread_info.priority);
  }
}

void PreemptFIFOScheduler::unblock_one_thread(Thread** pp_blocked_list_head) {
  Thread *p_thread = PreemptFIFOScheduler::p_active_thread;
  PreemptFIFOScheduler::move_node(pp_blocked_list_head, &ready_list_heads[p_thread->thread_info.priority], *pp_blocked_list_head);
  p_thread->thread_info.state = READY;
  set_bitpos<uint32_t>(prio_bitmap, p_thread->thread_info.priority);
}

