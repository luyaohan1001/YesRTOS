/**
 * @file thread.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS thread implementation.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include <thread.hpp>
#include <baremetal_api.h>

using namespace YesRTOS;

/**
 * @brief Construct a new Thread object
 * @param id Unique ID for the thread, defined by the user.
 * @param routine_ptr Function pointer to the execution routine.
 */
Thread::Thread(uint32_t id, void (*routine_ptr)(void)) {
  this->thread_info.id = id;
  this->thread_info.state = ACTIVE;
  this->set_routine(routine_ptr);

  uint32_t* contxt_stk_bottom = (uint32_t*)(&this->allocated_stack[0] + STACK_ALLOCATION_SIZE);
  this->stkptr = contxt_stk_bottom;

  // Initialize stack context for the thread to run for the first time.
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
 */
void Thread::init_stack() {
  init_stack_armv7m(&this->stkptr, (uint32_t*)(uintptr_t)this->thread_info.routine_ptr);
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
  this->thread_info.state = ACTIVE;
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
  this->thread_info.state = ACTIVE;
}

/**
 * @brief Put thread to sleep.
 */
void Thread::to_sleep() {
  this->thread_info.state = SLEEP;
}