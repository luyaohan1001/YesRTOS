#include "mutex.hpp"
#include "atomic_section.hpp"

using namespace YesRTOS;

Mutex::Mutex() {
  this->locked = 0;
  this->p_blocked_list = nullptr;
  this->owner = nullptr;
}

Mutex::~Mutex() {
}

/*
 * Lock the mutex.
 *
 * On a single core, disabling exceptions is enough to make "check" (is the lock free?)
 * and "update" (claim the lock) indivisible. It also protects the scheduler lists from
 * being modified by PendSV halfway through.
 *
 * If the mutex is taken, the running thread is moved to the blocked list and a context switch
 * is requested. PendSV is taken as soon as the atomic section ends, and this thread resumes only
 * after unlock() has handed ownership over to it.
 */
void Mutex::lock() {
  atomic_section a;

  if (!locked) {
    locked = 1;
    this->owner = PreemptFIFOScheduler::p_active_thread;
    return;
  }

  PreemptFIFOScheduler::block_running_thread(&this->p_blocked_list);
  request_context_switch();
}

/*
 * Unlock the mutex.
 *
 * Only the owning thread can unlock the mutex, as enforced by the `owner` check.
 *
 * If there are threads blocked on this mutex, ownership is handed over to the longest waiting
 * thread and `locked` stays set, so no other thread can take the mutex in between.
 * Otherwise, the mutex is simply marked as unlocked.
 */
void Mutex::unlock() {
  atomic_section a;

  // deny unlock for non-owner thread.
  if (PreemptFIFOScheduler::p_active_thread != this->owner) {
    return;
  }

  if (this->p_blocked_list) {
    this->owner = PreemptFIFOScheduler::unblock_one_thread(&this->p_blocked_list);
    request_context_switch();
  } else {
    locked = 0;
    this->owner = nullptr;
  }
}
