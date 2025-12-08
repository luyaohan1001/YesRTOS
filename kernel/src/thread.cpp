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

namespace YesRTOS {

Thread::Thread(uint32_t id, void (*routine_ptr)(void), uint8_t priority) {
  this->thread_info.id = id;
  this->thread_info.state = READY;
  this->thread_info.priority = priority;
  this->set_routine(routine_ptr);

  // Initialize thread routine stack (PSP).
  uint32_t* contxt_stk_bottom = (uint32_t*)(&this->allocated_stack[0] + STACK_ALLOCATION_SIZE);
  this->stkptr = contxt_stk_bottom;
  this->init_stack();
}

/**
 * @brief Destroy the Thread:: Thread object
 */
Thread::~Thread() {
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

bool Thread::operator==(const Thread& other) const {
  bool equal = (&this->allocated_stack[0] == &other.allocated_stack[0]) && (this->stkptr == other.stkptr) && (this->thread_info.id == other.thread_info.id) &&
               (this->thread_info.routine_ptr == other.thread_info.routine_ptr);
  return equal;
}

}  // namespace YesRTOS