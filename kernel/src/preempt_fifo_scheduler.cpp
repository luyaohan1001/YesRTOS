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

#include "atomic_section.hpp"
#include "bitops.hpp"

using namespace YesRTOS;

Thread* PreemptFIFOScheduler::p_active_thread = nullptr;

bool PreemptFIFOScheduler::init_complete = false;

static_assert(MAX_PRIO_LEVEL < 32, "prio_bitmap needs one bit per user priority level plus one for the idle thread");

Thread* PreemptFIFOScheduler::ready_list_heads[MAX_PRIO_LEVEL + 1] {nullptr};

uint32_t PreemptFIFOScheduler::prio_bitmap;

Thread* PreemptFIFOScheduler::completed_list = nullptr;

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
bool PreemptFIFOScheduler::add_thread(Thread* p_new) {
  // ready_list_heads has one entry per user level plus the idle level; anything else would index past it.
  if (!p_new || p_new->thread_info.priority >= MAX_PRIO_LEVEL) {
    return false;
  }

  // Once the scheduler runs, PendSV walks the same lists: keep it out while they are being linked.
  atomic_section a;
  if (!init_complete) PreemptFIFOScheduler::init();
  p_new->thread_info.state = READY;
  PreemptFIFOScheduler::insert_ready(p_new);
  PreemptFIFOScheduler::preempt_if_higher(p_new);
  return true;
}

/**
 * @brief Append a thread to the tail of the ready list of its priority (FIFO order).
 */
void PreemptFIFOScheduler::insert_ready(Thread* p_new) {
  uint8_t prio_level = p_new->thread_info.priority;
  PreemptFIFOScheduler::append_node(&ready_list_heads[prio_level], p_new);
  set_bitpos<uint32_t>(prio_bitmap, prio_level);
}

/**
 * @brief Request a context switch if a thread just made ready outranks the running one.
 * @note  Before start() there is no running thread; start() picks the highest priority itself.
 */
void PreemptFIFOScheduler::preempt_if_higher(Thread* p_ready) {
  if (p_active_thread && p_ready->thread_info.priority < p_active_thread->thread_info.priority) {
    request_context_switch();
  }
}

/**
 * @brief Move the running thread behind the other ready threads of its priority and let the first of them run.
 * @note  Under SCHED_FIFO this is the only way for equal priority threads to share the CPU without blocking.
 */
void PreemptFIFOScheduler::yield() {
  atomic_section a;
  Thread* p_thread = p_active_thread;
  Thread** pp_list = &ready_list_heads[p_thread->thread_info.priority];
  PreemptFIFOScheduler::unlink_node(pp_list, p_thread);
  PreemptFIFOScheduler::append_node(pp_list, p_thread);
  request_context_switch();
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
  PreemptFIFOScheduler::p_active_thread->thread_info.state = RUNNING;

#if defined (ARMV7M)
  itm_initialize();
  kernel_exception_priority_init();
  systick_clk_init();
  start_first_task();
#else
  #error "Timeslice not supported for undefined architecture."
#endif
}

/**
 * @brief Pick the thread to run: the head of the highest priority non-empty ready list (SCHED_FIFO).
 * @note  The running thread stays at the head of its ready list while it runs, so a thread preempted by a higher
 *        priority one resumes before the other threads of its priority. Equal priority threads are not rotated here:
 *        they only take turns when the running thread blocks, yields or exits. A SysTick therefore changes nothing
 *        unless a higher priority thread became ready.
 */
void PreemptFIFOScheduler::schedule_next() {
  uint32_t prio = count_trailing_zero<uint32_t>(prio_bitmap);
  Thread* p_next_ready = ready_list_heads[prio];

  if (p_active_thread->thread_info.state == RUNNING) {
    p_active_thread->thread_info.state = READY;
  }
  p_next_ready->thread_info.state = RUNNING;
  PreemptFIFOScheduler::p_active_thread = p_next_ready;
}

/**
 * @brief Unlink a node from a doubly linked list.
 */
void PreemptFIFOScheduler::unlink_node(Thread** pp_list, Thread* node) {
  Thread* p_prev = node->thread_info.p_prev;
  Thread* p_next = node->thread_info.p_next;
  if (p_prev) {
    p_prev->thread_info.p_next = p_next;
  } else {
    *pp_list = p_next;
  }
  if (p_next) p_next->thread_info.p_prev = p_prev;
  node->thread_info.p_next = nullptr;
  node->thread_info.p_prev = nullptr;
}

/**
 * @brief Append a node at the tail of a doubly linked list.
 */
void PreemptFIFOScheduler::append_node(Thread** pp_list, Thread* node) {
  node->thread_info.p_next = nullptr;
  if (!*pp_list) {
    node->thread_info.p_prev = nullptr;
    *pp_list = node;
    return;
  }
  Thread* p_tail = *pp_list;
  while (p_tail->thread_info.p_next) p_tail = p_tail->thread_info.p_next;
  p_tail->thread_info.p_next = node;
  node->thread_info.p_prev = p_tail;
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

void PreemptFIFOScheduler::exit_running_thread() {
  {
    atomic_section a;
    Thread* p_thread = p_active_thread;
    uint8_t prio_level = p_thread->thread_info.priority;
    PreemptFIFOScheduler::move_node(&ready_list_heads[prio_level], &completed_list, p_thread);
    p_thread->thread_info.state = COMPLETE;
    if (ready_list_heads[prio_level] == nullptr) {
      clr_bitpos<uint32_t>(prio_bitmap, prio_level);
    }
    request_context_switch();
  }
  // PendSV is taken as soon as the atomic section ends. This thread is in no ready list, so it never resumes here.
  while (1) {
  }
}

/**
 * @brief Where a thread routine returns to: its initial LR (see init_stack_armv7m()).
 */
extern "C" void yesrtos_thread_exit(void) {
  PreemptFIFOScheduler::exit_running_thread();
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

  PreemptFIFOScheduler::unlink_node(pp_blocked_list_head, p_thread);
  p_thread->thread_info.state = READY;
  PreemptFIFOScheduler::insert_ready(p_thread);  // behind the ready threads of its priority
  PreemptFIFOScheduler::preempt_if_higher(p_thread);
  return p_thread;
}

