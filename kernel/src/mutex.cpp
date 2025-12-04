#include "mutex.hpp"
#include "atomic_section.hpp"

using namespace YesRTOS;

mutex::mutex() {
    this->locked.store(false);
    this->p_blocked_list = nullptr;
}

mutex::~mutex() {
}

void mutex::lock() {
  // check if mutex has been taken.
  if (locked == true) {
    PreemptFIFOScheduler::p_active_thread->thread_info.state = BLOCKED;
    PreemptFIFOScheduler::move_node(&PreemptFIFOScheduler::ready_list, &this->p_blocked_list, (Thread *)PreemptFIFOScheduler::p_active_thread);
  }
  request_context_switch();

  // block current thread until lock is released.
  while(locked == true);

  // take mutex.
  locked = true;
  this->owner = PreemptFIFOScheduler::p_active_thread;
}

void mutex::unlock() {
  // deny unlock for non-owner thread.
  if (PreemptFIFOScheduler::p_active_thread != this->owner) {
    return;
  }
  // unlock mutex.
  locked = false;

  // move other blocked thread to ready list.
  if (this->p_blocked_list) {
    PreemptFIFOScheduler::move_node(&this->p_blocked_list, &PreemptFIFOScheduler::ready_list, (Thread *)this->p_blocked_list);
    this->p_blocked_list->thread_info.state = ACTIVE;
  }
  request_context_switch();
}


