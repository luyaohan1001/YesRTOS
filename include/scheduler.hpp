#pragma once
#include <unistd.h>

#include <csignal>
#include <iostream>
#include <thread.hpp>
#include <vector>

namespace YesRTOS {
class RoundRobinScheduler {
 public:
  RoundRobinScheduler();
  ~RoundRobinScheduler();

  static void start();
  static void add_thread(Thread thread);
  static void kernel_interrupt();
  static void schedule();

 private:
  static long tick;
  static std::vector<YesRTOS::Thread> thread_q;
};
}  // namespace YesRTOS
