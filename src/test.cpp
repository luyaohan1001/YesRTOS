#include <iostream>
#include <scheduler.hpp>

using namespace YesRTOS;

void(thread0_routine)(Thread& thread_handle) {
  for (auto i = 0; i < 5; ++i) {
    std::cout << "️♥ Thread 0 ♥️" << std::endl;
  }
}

void(thread1_routine)(Thread& thread_handle) {
  for (auto i = 0; i < 5; ++i) {
    std::cout << "️♥ Thread 1 ♥️" << std::endl;
  }
}

int main(int argc, char const* argv[]) {
  // main function --> actual entry.
  Thread thread0(thread_info_t{.id = 0, .routine_ptr = thread0_routine});
  RoundRobinScheduler::add_thread(thread0);

  Thread thread1(thread_info_t{.id = 1, .routine_ptr = thread1_routine});
  RoundRobinScheduler::add_thread(thread1);

  RoundRobinScheduler::schedule();
  return 0;
}
