/**
 * @file test.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Providing entry to a main() function for exercising YesRTOS code.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include <baremetal_api.h>
#include <iostream>
#include <scheduler.hpp>

using namespace YesRTOS;

/**
 * @brief Thread0 routine.
 */
void(thread0_routine)() {
  while (1) {
    // std::cout << "️♥ Thread 0 ♥️" << std::endl;
    // itm_trace("Thread 0\n");
    int a = 15;
    int b = 13;
    int c = 14;
    int d = 11;
  }
}

/**
 * @brief Thread1 routine.
 */
void(thread1_routine)() {
  while (1) {
    // itm_trace("Thread 1\n");
    int a = 10;
    // std::cout << "️♥ Thread 1 ♥️" << std::endl;
  }
}

/**
 * @brief Thread1 routine.
 */
void(thread2_routine)() {
  while (1) {
    std::cout << "️♥ Thread 2 ♥️" << std::endl;
  }
}

/**
 * @brief YesRTOS Cooperative style scheduling demo between two threads.
 */
int main() {
  // Get schedueler instance.
  extern YesRTOS::PreemptiveScheduler sched;

  // Add threads.
  Thread thread0(0, thread0_routine);
  sched.add_thread(&thread0);

  Thread thread1(1, thread1_routine);
  sched.add_thread(&thread1);

  // Thread thread2(2, thread2_routine);
  // sched.add_thread(&thread2);

  sched.start();
  while (1);
  return 0;
}