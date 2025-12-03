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
    bool expected = false;
    bool st = locked.compare_exchange_strong(expected, true, std::memory_order_acquire);
    if (!st) {
      // lock failed, yield to other thread.
      PreemptFIFOScheduler::p_active_thread->thread_info.state = BLOCKED;
      request_context_switch();
    }
}


void mutex::unlock() {
  // the current thread own the lock, so it will only execute once.
  locked.store(false, std::memory_order_release);
  // PreemptFIFOScheduler::move_node(&this->p_blocked_list, &PreemptFIFOScheduler::ready_list, (Thread *)PreemptFIFOScheduler::p_active_thread);
}


