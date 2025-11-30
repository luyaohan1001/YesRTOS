/**
 * @file test.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Providing entry to a main() function for exercising YesRTOS code.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include "baremetal_api.h"
#include "preempt_fifo_scheduler.hpp"
#include "rr_scheduler.hpp"
#include "spinlock.hpp"
#include "atomic_section.hpp"
#include <atomic>

#if defined (HOST_PLATFORM)
#include <iostream>
#endif

using namespace YesRTOS;

// declare lock to protect tracer critical section.
YesRTOS::spinlock tracer_lock;



/**
 * @brief Thread0 routine.
 */
void(thread0_routine)() {
  while (1) {
    {
      {
      tracer_lock.lock();
      // atomic_section a;
      itm_trace("thread 0\n");
      tracer_lock.unlock();
      }
    }
  }
}

/**
 * @brief Thread1 routine.
 */
void(thread1_routine)() {
  while (1) {
    {
      {
      // atomic_section a;
      tracer_lock.lock();
      itm_trace("thread 1\n");
      tracer_lock.unlock();
      }
    }
  }
}

/**
 * @brief Thread2 routine.
 */
void(thread2_routine)() {
  while (1) {
    {
      tracer_lock.lock();
      // atomic_section a;
      itm_trace("thread 2\n");
      tracer_lock.unlock();
    }
  }
}

/**
 * @brief Thread3 routine.
 */
void(thread3_routine)() {
  while (1) {
    {
      // atomic_section a;
      tracer_lock.lock();
      itm_trace("thread 3\n");
      tracer_lock.unlock();
    }
  }
}

/**
 * @brief YesRTOS Cooperative style scheduling demo between two threads.
 */
int main() {
  Thread thread0(0, thread0_routine);
  PreemptFIFOScheduler::add_thread(&thread0, 4);

  Thread thread1(1, thread1_routine);
  PreemptFIFOScheduler::add_thread(&thread1, 4);

  Thread thread2(1, thread2_routine);
  PreemptFIFOScheduler::add_thread(&thread2, 4);

  Thread thread3(1, thread3_routine);
  PreemptFIFOScheduler::add_thread(&thread3, 4);

  PreemptFIFOScheduler::start();

  while (1);
  return 0;
}
