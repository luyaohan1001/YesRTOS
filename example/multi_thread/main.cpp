/**
 * @file test.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Providing entry to a main() function for exercising YesRTOS code.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

// #include "baremetal_api.h"
#include "preempt_fifo_scheduler.hpp"
#include "rr_scheduler.hpp"
#include "mutex.hpp"
#include "atomic_section.hpp"

#if defined (HOST_PLATFORM)
#include <iostream>
#endif

using namespace YesRTOS;

// declare lock to protect tracer critical section.
// YesRTOS::mutex tracer_lock;


/**
 * @brief Thread0 routine.
 */
void(thread0_routine)() {
  while (1) {
    {
      {
      // atomic_section a;
      // itm_trace("thread 0\n");
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
      // itm_trace("thread 1\n");
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
      // atomic_section a;
      // itm_trace("thread 2\n");
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
      // itm_trace("thread 3\n");
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