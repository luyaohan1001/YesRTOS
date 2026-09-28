/**
 * @file thread.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS thread implementation.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */
#if defined(ARMV7M)
#include <baremetal_api.h>
#endif

#include <thread.hpp>
#include <atomic_section.hpp>
#include <preempt_fifo_scheduler.hpp>

#if defined(ARMV7M)
extern "C" volatile uint32_t yesrtos_scheduler_started;  // timeslice.cpp: non-zero once the first thread runs.
#endif

namespace YesRTOS {

Thread* Thread::registry_head = nullptr;

Thread::Thread(uint32_t id, void (*routine_ptr)(void), uint8_t priority) {
  this->thread_info.id = id;
  this->thread_info.state = READY;
  this->thread_info.priority = priority;
  this->set_routine(routine_ptr);

  // Paint the stack, so stack_peak_bytes() can tell which words were ever written.
  for (uint32_t i = 0; i < STACK_ALLOCATION_SIZE; i++) {
    this->allocated_stack[i] = STACK_PAINT;
  }

  // Initialize thread routine stack (PSP).
  uint32_t* contxt_stk_bottom = (uint32_t*)(&this->allocated_stack[0] + STACK_ALLOCATION_SIZE);
  this->stkptr = contxt_stk_bottom;
  this->init_stack();

  atomic_section a;
  this->p_registry_next = registry_head;
  registry_head = this;
}

/**
 * @brief Destroy the Thread:: Thread object
 */
Thread::~Thread() {
  {
    atomic_section a;
    for (Thread** pp = &registry_head; *pp; pp = &(*pp)->p_registry_next) {
      if (*pp == this) {
        *pp = this->p_registry_next;
        break;
      }
    }
  }
  this->thread_info.routine_ptr = nullptr;
  this->thread_info.state = COMPLETE;
}

/**
 * @brief Initialize thread stack for necessary context.
 * @note  This function is a wrapper call for any architecture dependent thread stack initialization.
 *        The ARMV7M switch also strips away machine dependent function for unit testing higher level data structure explicitly instantiate 'Thread' class for unit testing.
 */
void Thread::init_stack() {
#if defined(ARMV7M)
  init_stack_armv7m(&this->stkptr, (uint32_t*)(uintptr_t)this->thread_info.routine_ptr);
#endif
}

const thread_state_t& Thread::get_state() const {
  return this->thread_info.state;
}

/**
 * @brief Set the thread rountine.
 * @param routine_ptr
 */
void Thread::set_routine(void (*routine_ptr)(void)) {
  this->thread_info.routine_ptr = routine_ptr;
  this->thread_info.state = READY;
}

/**
 * @brief Run thread.
 */
void Thread::run() {
  this->thread_info.state = RUNNING;
  (*this->thread_info.routine_ptr)();
  this->thread_info.state = COMPLETE;
}

/**
 * @brief Set the lifecycle state of current thread.
 * @param cfg
 */
void Thread::set_state(thread_state_t cfg) {
  this->thread_info.state = cfg;
}

/**
 * @brief Wake up thread from sleep.
 */
void Thread::wake_up() {
  this->thread_info.state = READY;
}

/**
 * @brief Put thread to sleep.
 */
void Thread::to_sleep() {
  this->thread_info.state = SLEEP;
}

uint32_t Thread::stack_size_bytes() const {
  return STACK_ALLOCATION_SIZE * sizeof(uint32_t);
}

uint32_t Thread::stack_used_bytes() const {
  uintptr_t sp = (uintptr_t)this->stkptr;
#if defined(ARMV7M)
  // The running thread's stkptr is only updated when it is switched out; its live stack pointer is PSP.
  if (yesrtos_scheduler_started && this == PreemptFIFOScheduler::p_active_thread) {
    uint32_t psp;
    __asm volatile("mrs %0, psp" : "=r"(psp));
    sp = psp;
  }
#endif
  uintptr_t top = (uintptr_t)&this->allocated_stack[STACK_ALLOCATION_SIZE];
  uintptr_t bottom = (uintptr_t)&this->allocated_stack[0];
  if (sp >= top) return 0;
  if (sp < bottom) return this->stack_size_bytes();  // stack pointer below the stack: overflowed.
  return (uint32_t)(top - sp);
}

uint32_t Thread::stack_peak_bytes() const {
  // The stack grows down from the end of allocated_stack: untouched words are left at the start.
  uint32_t untouched = 0;
  while (untouched < STACK_ALLOCATION_SIZE && this->allocated_stack[untouched] == STACK_PAINT) {
    untouched++;
  }
  uint32_t peak = (STACK_ALLOCATION_SIZE - untouched) * sizeof(uint32_t);
  // Stack reserved but not written (e.g. the initial r0-r3, r12, r4-r11 slots) still holds the paint: the current use
  // is a lower bound too.
  uint32_t used = this->stack_used_bytes();
  return peak > used ? peak : used;
}

bool Thread::operator==(const Thread& other) const {
  bool equal = (&this->allocated_stack[0] == &other.allocated_stack[0]) && (this->stkptr == other.stkptr) && (this->thread_info.id == other.thread_info.id) &&
               (this->thread_info.routine_ptr == other.thread_info.routine_ptr);
  return equal;
}

}  // namespace YesRTOS