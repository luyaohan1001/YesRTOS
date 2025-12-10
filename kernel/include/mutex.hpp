#pragma once

#include <atomic>
#include "thread.hpp"
#include "baremetal_api.h"
#include "preempt_fifo_scheduler.hpp"

namespace YesRTOS {

/**
 * @brief Mutex (Mutual Exclusion Lock) for thread synchronization.
 *
 * This mutex guarantees that only one thread can enter a critical section
 * at a time. If the mutex is already locked, the calling thread will be
 * blocked and placed into the mutex's blocked list until the mutex becomes
 * available.
 *
 * This mutex is designed for preemptive RTOS environments and supports:
 *  - Blocking lock (no spinning)
 *  - Thread ownership tracking
 *  - Safe unlock by owner only
 */
class Mutex {
public:
    /**
     * @brief Construct a new mutex object.
     *
     * Initializes the mutex to an unlocked state.
     * No thread owns the mutex at creation time.
     * The blocked list is initially empty.
     */
    Mutex();

    /**
     * @brief Destroy the mutex object.
     *
     * The mutex must NOT be destroyed while still locked.
     * Destroying a locked mutex leads to undefined behavior.
     */
    ~Mutex();

    /**
     * @brief Acquire the mutex lock.
     *
     * If the mutex is currently unlocked, the calling thread becomes
     * the new owner and continues execution immediately.
     *
     * If the mutex is already locked, the calling thread is:
     *  - Marked as BLOCKED
     *  - Inserted into the mutex blocked list
     *  - A context switch is triggered to allow another thread to run
     *
     * This call will block until the mutex becomes available.
     */
    void lock();

    /**
     * @brief Release the mutex lock.
     *
     * Only the owning thread is allowed to unlock the mutex.
     *
     * If there are waiting threads in the blocked list:
     *  - One waiting thread is selected (FIFO depending on implementation)
     *  - That thread becomes the new owner
     *  - That thread is moved to the scheduler ready list
     *
     * If no threads are waiting:
     *  - The mutex becomes unlocked
     *  - The owner is cleared
     */
    void unlock();

public:
    /** 
     * @brief Atomic flag indicating whether the mutex is locked.
     * 
     * true  -> locked
     * false -> unlocked
     */
    std::atomic<bool> locked;

    /**
     * @brief Linked list of threads blocked on this mutex.
     *
     * Threads attempting to lock while the mutex is locked
     * are inserted into this list.
     */
    YesRTOS::Thread* p_blocked_list;

    /**
     * @brief Pointer to the thread that currently owns the mutex.
     *
     * Only this thread is permitted to unlock the mutex.
     * Used for:
     *  - Ownership validation
     *  - Priority inheritance
     *  - Debugging and deadlock detection
     */
    YesRTOS::Thread* owner;
};

} // namespace YesRTOS
