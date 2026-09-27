#pragma once

#include <cstdint>

#include "baremetal_api.h"
#include "preempt_fifo_scheduler.hpp"
#include "thread.hpp"

namespace YesRTOS {

/**
 * @brief Counting semaphore.
 *
 * Holds up to `max` tokens. acquire() takes one, blocking while none is available; release() gives one back, or hands
 * it straight to a blocked thread. Typical uses: counting free slots or items, and signalling a thread from an
 * interrupt handler (release() and try_acquire() are safe there; acquire() is not).
 */
class Semaphore {
public:
    /**
     * @param initial Tokens available at start (clamped to max).
     * @param max     Largest count; release() beyond it fails. Defaults to unbounded.
     */
    explicit Semaphore(uint32_t initial = 0, uint32_t max = UINT32_MAX);

    /**
     * @brief Take a token, blocking the calling thread until one is available.
     * @note  Threads only, and not inside a critical section: it stops in an infinite loop (visible in a debugger) when
     *        called from an interrupt handler or with exceptions disabled, where blocking is impossible.
     */
    void acquire();

    /**
     * @brief Take a token if one is available, without blocking. Safe in interrupt handlers.
     * @return false if the count was 0.
     */
    bool try_acquire();

    /**
     * @brief Give a token back. Safe in interrupt handlers.
     *
     * If threads are blocked in acquire(), the token goes directly to the longest waiting one (the count does not
     * change) and it preempts the caller if it has a higher priority; from an interrupt handler that happens as soon as
     * the handler returns.
     *
     * @return false, with nothing changed, if the count is already at max.
     */
    bool release();

    /**
     * @brief Tokens currently available.
     */
    uint32_t count() const;

private:
    /**
     * @brief Available tokens. Taken by CAS on the fast path; changed by plain stores only with exceptions disabled.
     */
    volatile uint32_t available;

    uint32_t max_count;

    /**
     * @brief Threads blocked in acquire(), most recent first.
     */
    YesRTOS::Thread* p_blocked_list;
};

}  // namespace YesRTOS
