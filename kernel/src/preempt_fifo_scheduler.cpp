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

static_assert(MAX_PRIO_LEVEL < 32, "prio_bitmap needs one bit per user priority level plus one for the idle thread");

Thread* PreemptFIFOScheduler::ready_list_heads[MAX_PRIO_LEVEL + 1] {nullptr};

uint32_t PreemptFIFOScheduler::prio_bitmap;

extern "C" __attribute__((weak)) void yesrtos_idle_hook(void) {
}

/**
 * @brief Idle thread routine: always ready at the lowest priority, so the scheduler always has a thread to pick, even
 *        when every user thread is blocked.
 */
static void idle_routine() {
  while (1) {
    yesrtos_idle_hook();
    __asm volatile("wfi");  // sleep until the next interrupt (e.g. SysTick) instead of spinning.
  }
}

static Thread idle_thread(UINT32_MAX, idle_routine, PreemptFIFOScheduler::IDLE_PRIO);

void PreemptFIFOScheduler::init() {
  PreemptFIFOScheduler::init_complete = true;
}

/**
 * @brief Add thread to scheduler queue.
 */
void PreemptFIFOScheduler::add_thread(Thread* p_new) {
  if (!init_complete) PreemptFIFOScheduler::init();
  PreemptFIFOScheduler::insert_ready(p_new);
}

/**
 * @brief Insert a thread at the head of the ready list of its priority.
 */
void PreemptFIFOScheduler::insert_ready(Thread* p_new) {
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

  // The idle thread keeps prio_bitmap non-zero: count_trailing_zero(0) is undefined.
  PreemptFIFOScheduler::insert_ready(&idle_thread);

  uint32_t prio = count_trailing_zero<uint32_t>(prio_bitmap);
  PreemptFIFOScheduler::p_active_thread = ready_list_heads[prio];

#if defined (ARMV7M)
  itm_initialize();
  kernel_exception_priority_init();
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
  // A blocked thread's links point into a blocked list, so only rotate from the active thread while it is still ready.
  if (prio == p_active_thread->thread_info.priority && p_active_thread->thread_info.state != BLOCKED) {
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

Thread* PreemptFIFOScheduler::unblock_one_thread(Thread** pp_blocked_list_head) {
  Thread *p_thread = *pp_blocked_list_head;
  if (!p_thread) {
    return nullptr;
  }

  // Blocked threads are inserted at the head, so the tail is the longest waiting thread.
  while (p_thread->thread_info.p_next) {
    p_thread = p_thread->thread_info.p_next;
  }

  uint8_t prio_level = p_thread->thread_info.priority;
  PreemptFIFOScheduler::move_node(pp_blocked_list_head, &ready_list_heads[prio_level], p_thread);
  p_thread->thread_info.state = READY;
  set_bitpos<uint32_t>(prio_bitmap, prio_level);
  return p_thread;
}

