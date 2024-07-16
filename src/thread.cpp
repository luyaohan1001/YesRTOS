/**
 * @file thread.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS thread implementation.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include <thread.hpp>

using namespace YesRTOS;

/**
 * @brief Construct a new Thread:: Thread object
 * @param id Unique ID for the thread, defined by the user.
 * @param routine_ptr Function pointer to the execution routine.
 */
Thread::Thread(uint32_t id, void (*routine_ptr)(Thread* self)) {
  this->thread_info.id = id;
  this->thread_info.state = ACTIVE;
  this->set_rountine(routine_ptr);
}

/**
 * @brief Destroy the Thread:: Thread object
 */
Thread::~Thread() {
  this->thread_info.routine_ptr = nullptr;
  this->thread_info.state = COMPLETE;
}

/**
 * @brief Get current thread lifecycle state.
 * @return const thread_state_t&
 */
const thread_state_t& Thread::get_state() const {
  return this->thread_info.state;
}

/**
 * @brief Set the thread rountine.
 * @param routine_ptr
 */
void Thread::set_rountine(void (*routine_ptr)(Thread* thread_handle)) {
  this->thread_info.routine_ptr = routine_ptr;
  this->thread_info.state = ACTIVE;
}

/**
 * @brief Run thread.
 */
void Thread::run() {
  this->thread_info.state = RUNNING;
  (*this->thread_info.routine_ptr)(this);
  this->thread_info.state = COMPLETE;
}

/**
 * @brief Get the stack pointer storing current thread's context.
 * @return uint32_t*
 */
uint32_t* Thread::get_thread_stack_ptr() {
  return this->thread_stack_ptr;
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