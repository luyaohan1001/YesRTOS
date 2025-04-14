/**
 * @file test.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Providing entry to a main() function for exercising YesRTOS code.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include <baremetal_api.h>
#include <preempt_fifo_scheduler.hpp>
#include <rr_scheduler.hpp>

#if defined (HOST_PLATFORM)
#include <iostream>
#endif

using namespace YesRTOS;

/**
 * @brief Thread0 routine.
 */
void(thread0_routine)() {
  while (1) {
  }
}

/**
 * @brief Thread1 routine.
 */
void(thread1_routine)() {
  while (1) {
  }
}

/**
 * @brief Thread1 routine.
 */
void(thread2_routine)() {
  while (1) {
  }
}

/**
 * @brief YesRTOS Cooperative style scheduling demo between two threads.
 */
int main() {
  // Add threads.
  // Thread thread0(0, thread0_routine);
  // RoundRobinScheduler::add_thread(&thread0);

  // Thread thread1(1, thread1_routine);
  // RoundRobinScheduler::add_thread(&thread1);

  // Thread thread2(2, thread2_routine);
  // RoundRobinScheduler::add_thread(&thread2);

  // RoundRobinScheduler::start();

  Thread thread0(0, thread0_routine);
  PreemptFIFOScheduler::add_thread(&thread0, 0);

  Thread thread1(1, thread1_routine);
  PreemptFIFOScheduler::add_thread(&thread1, 0);

  Thread thread2(2, thread2_routine);
  PreemptFIFOScheduler::add_thread(&thread2, 0);

  PreemptFIFOScheduler::start();

  while (1);
  return 0;
}