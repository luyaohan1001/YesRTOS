/**
 * @file test.cpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Providing entry to a main() function for exercising YesRTOS code.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#include <armv7m.h>

#include <iostream>
#include <scheduler.hpp>

#define DBG

using namespace YesRTOS;

// Initialize thread on DATA for testing references.
#ifdef DBG
Thread *thread0_ptr, *thread1_ptr;
#endif

/**
 * @brief Custom routine for Thread0: some random operations followed with yield call..
 */
void(thread0_routine)(Thread* thread_handle) {
  for (auto i = 0; i < 5; ++i) {
    std::cout << "️♥ Thread 0 ♥️" << std::endl;
    int a = 15;
    int b = 13;
    int c = 14;
    int d = 11;
    // wake up thread 1
    thread1_ptr->wake_up();
    // set current to sleep and yield current thread
    thread0_ptr->to_sleep();
    yield_thread(thread_handle->thread_stack_ptr);
    a = a + i;
    a *= (a + b + c + d);
  }
}

/**
 * @brief Custom routine for Thread1: yield to thread 0 on first time entry.
 */
void(thread1_routine)(Thread* thread_handle) {
  for (auto i = 0; i < 5; ++i) {
    std::cout << "️♥ Thread 1 ♥️" << std::endl;
    // Use flag to make thread1 yeild to thread0 on first entry.
    static bool first_entry = false;
    if (first_entry == false) {
      first_entry = true;
      // wake up thread0
      thread0_ptr->wake_up();
      // set current to sleep and yield
      thread1_ptr->to_sleep();
      yield_thread(thread_handle->thread_stack_ptr);
    }
  }
}

/**
 * @brief YesRTOS Cooperative style scheduling demo between two threads.
 */
int main() {
  RoundRobinScheduler sched0;

  Thread thread0(0, thread0_routine);
  sched0.add_thread(&thread0);

  Thread thread1(1, thread1_routine);
  sched0.add_thread(&thread1);

#ifdef DBG
  thread0_ptr = &thread0;
  thread1_ptr = &thread1;
#endif
  sched0.schedule();
  return 0;
}