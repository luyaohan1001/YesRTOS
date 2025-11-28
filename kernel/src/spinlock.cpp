#include "spinlock.hpp"
#include <atomic>

namespace YesRTOS {

spinlock::spinlock() {
    this->locked.store(false);
}

spinlock::~spinlock() {
}

void spinlock::lock() {
    bool expected = false;
    // Try to change false -> true
    while (!locked.compare_exchange_strong(expected, true, std::memory_order_acquire)) {
        expected = false;  // must reset expected after failure
    }
}


void spinlock::unlock() {
  // the current thread own the lock, so it will only execute once.
  locked.store(false, std::memory_order_release);
}

}  // namespace YesRTOS
