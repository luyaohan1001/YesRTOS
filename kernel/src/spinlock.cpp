#include "spinlock.hpp"

namespace YesRTOS {

spinlock::spinlock() {
  this->locked = 0;
}

spinlock::~spinlock() {
}

void spinlock::lock() {
  // Try to change 0 -> 1. A failed CAS means another thread holds the lock: keep spinning until it is released.
  while (!atomic_compare_and_swap(&this->locked, 0, 1)) {
  }
}

void spinlock::unlock() {
  // the current thread own the lock, so a plain store is enough to release it.
  // DMB first so the critical section's memory accesses complete before the lock is seen as free (release semantics).
  __asm volatile("dmb" ::: "memory");
  this->locked = 0;
}

}  // namespace YesRTOS
