#include <thread.hpp>

using namespace YesRTOS;

/// @brief Constructor.
/// @param cfg
Thread::Thread(thread_info_t cfg) {
  this->thread_info.id = cfg.id;
  this->set_rountine(cfg.routine_ptr);
}

/// @brief Destructor.
Thread::~Thread() {
  this->thread_info.routine_ptr = nullptr;
  this->thread_info.state = COMPLETE;
}

/// @brief Get thread state.
/// @return thread state reference.
const thread_state_t& Thread::get_state() const {
  return this->thread_info.state;
}

/// @brief Set thread routine.
/// @param routine_ptr
void Thread::set_rountine(void (*routine_ptr)(Thread& thread_handle)) {
  this->thread_info.routine_ptr = routine_ptr;
  this->thread_info.state = ACTIVE;
}

/// @brief Run thread.
void Thread::run() {
  this->thread_info.state = RUNNING;
  (*this->thread_info.routine_ptr)(*this);
  if (this->thread_info.state != SLEEP) {
    this->thread_info.state = COMPLETE;
  }
}

// Work in progress.
void Thread::yield() {
  this->thread_info.yield_request = true;
  // this->thread_info.state = SLEEP;
  // this->ctx.save_context();
}

// Work in progress.
void Thread::resume() {
  this->thread_info.state = ACTIVE;
}
