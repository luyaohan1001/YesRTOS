#include "semaphore.hpp"

#include "atomic_section.hpp"

using namespace YesRTOS;

Semaphore::Semaphore(uint32_t initial, uint32_t max) {
  this->max_count = max;
  this->available = initial < max ? initial : max;
  this->p_blocked_list = nullptr;
}

/*
 * Fast path: take a token with a CAS while the count is non-zero, exceptions stay enabled.
 *
 * Slow path: with exceptions disabled no other thread or handler runs on this single core, so "is there a token?" and
 * "block" become indivisible and release() cannot slip in between (no lost wake-up). A token released between the
 * failed fast path and the section is taken here. Otherwise the thread blocks; PendSV is taken as soon as the section
 * ends, and the thread resumes only once release() has handed it a token.
 */
void Semaphore::acquire() {
  if (this->try_acquire()) {
    return;
  }

  atomic_section a;

  if (this->available > 0) {
    this->available = this->available - 1;
    return;
  }

  if (a.nested() || in_exception_handler()) {
    // Blocking is impossible here: the context switch could not be taken before returning and the caller would carry
    // on without a token. Stop (visible in a debugger) instead of breaking the count.
    while (1) {
    }
  }

  PreemptFIFOScheduler::block_running_thread(&this->p_blocked_list);
  request_context_switch();
}

bool Semaphore::try_acquire() {
  uint32_t current;
  do {
    current = this->available;
    if (current == 0) {
      return false;
    }
  } while (!atomic_compare_and_swap(&this->available, current, current - 1));
  return true;
}

/*
 * Always a (short, bounded) critical section: whether a thread waits and what to do about it must be decided
 * atomically with respect to acquire()'s slow path.
 */
bool Semaphore::release() {
  atomic_section a;

  if (this->p_blocked_list) {
    // Hand the token over: the woken thread returns from acquire() owning it. unblock_one_thread() requests a switch
    // if the thread outranks the running one; from a handler the switch happens when the handler returns.
    PreemptFIFOScheduler::unblock_one_thread(&this->p_blocked_list);
    return true;
  }

  if (this->available >= this->max_count) {
    return false;
  }
  this->available = this->available + 1;
  return true;
}

uint32_t Semaphore::count() const {
  return this->available;
}
