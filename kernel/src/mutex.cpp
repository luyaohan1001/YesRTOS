#include "mutex.hpp"
#include "atomic_section.hpp"

using namespace YesRTOS;

Mutex::Mutex() {
  this->locked = UNLOCKED;
  this->p_blocked_list = nullptr;
  this->owner = nullptr;
}

Mutex::~Mutex() {
}

/*
 * Lock the mutex.
 *
 * Fast path: an uncontended mutex is taken with a single CAS (UNLOCKED -> LOCKED), leaving exceptions enabled.
 *
 * Slow path: with exceptions disabled no other thread can run on this single core, so "check" and "block" become
 * indivisible, and PendSV cannot run while the scheduler lists are modified. The mutex may have been released between
 * the failed CAS and the section, so check again. Otherwise mark it LOCKED_CONTENDED, so the owner's unlock() takes
 * its slow path and hands ownership over, then block. PendSV is taken as soon as the section ends; this thread resumes
 * only after unlock() has made it the owner.
 */
void Mutex::lock() {
  if (atomic_compare_and_swap(&this->locked, UNLOCKED, LOCKED)) {
    this->owner = PreemptFIFOScheduler::p_active_thread;
    return;
  }

  atomic_section a;

  if (this->locked == UNLOCKED) {
    this->locked = LOCKED;
    this->owner = PreemptFIFOScheduler::p_active_thread;
    return;
  }

  if (a.nested()) {
    // Called with exceptions already disabled: the context switch could not be taken before returning, and the caller
    // would carry on without owning the mutex. Blocking calls are not allowed inside a critical section; stop here
    // (visible in a debugger) instead of breaking mutual exclusion.
    while (1) {
    }
  }

  this->locked = LOCKED_CONTENDED;
  PreemptFIFOScheduler::block_running_thread(&this->p_blocked_list);
  request_context_switch();
}

/*
 * Unlock the mutex.
 *
 * Only the owning thread can unlock the mutex, as enforced by the `owner` check.
 *
 * Fast path: nobody waits (LOCKED), release with a single CAS (LOCKED -> UNLOCKED). `owner` is cleared first: once the
 * CAS succeeds another thread may take the mutex and set its own owner.
 *
 * Slow path: the CAS fails because a waiter marked the mutex LOCKED_CONTENDED. Ownership is handed over to the longest
 * waiting thread and the mutex stays locked, so no other thread can take it in between; it drops back to LOCKED when
 * no other thread is left waiting.
 */
void Mutex::unlock() {
  if (PreemptFIFOScheduler::p_active_thread != this->owner) {
    return;
  }

  this->owner = nullptr;
  if (atomic_compare_and_swap(&this->locked, LOCKED, UNLOCKED)) {
    return;
  }

  atomic_section a;

  if (!this->p_blocked_list) {
    this->locked = UNLOCKED;
    return;
  }

  this->owner = PreemptFIFOScheduler::unblock_one_thread(&this->p_blocked_list);
  if (!this->p_blocked_list) {
    this->locked = LOCKED;
  }
  request_context_switch();
}
