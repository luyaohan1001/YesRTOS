#include "mutex.hpp"

mutex::mutex() {
    this->locked.store(false);
}

mutex::~mutex() {
}

void mutex::lock() {
    bool expected = false;
    // Try to change false -> true
    while (!locked.compare_exchange_strong(expected, true, std::memory_order_acquire)) {
        expected = false;  // must reset expected after failure
    }
}


void mutex::unlock() {
  // the current thread own the lock, so it will only execute once.
  locked.store(false, std::memory_order_release);
}


