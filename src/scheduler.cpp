#include <scheduler.hpp>

using namespace YesRTOS;

// Static Class Member Initialization
/// @brief Thread queue definition.
/// @details static (as storage modifier) to maintain single per class, and to be accessed by kernal interrupt which is static method.
/// @details other static keyword usage (as linkage modifier) does not apply here.
std::vector<YesRTOS::Thread> RoundRobinScheduler::thread_q;

/// @brief Tick counter.
long RoundRobinScheduler::tick = 0;

/// @brief Constructor.
RoundRobinScheduler::RoundRobinScheduler() {
}

/// @brief Destructor.
RoundRobinScheduler::~RoundRobinScheduler() {
  thread_q.clear();
}

/// @brief Invoke scheduler.
void RoundRobinScheduler::schedule() {
  while (true) {
    for (auto thread_ptr = RoundRobinScheduler::thread_q.begin(); thread_ptr != RoundRobinScheduler::thread_q.end(); thread_ptr++) {
      switch (thread_ptr->get_state()) {
        case ACTIVE:
          thread_ptr->run();
          break;

        case RUNNING:
          break;

        case BLOCKED:
          break;

        case SLEEP:
          thread_ptr->resume();
          break;

        case COMPLETE:
          break;
      }
    }
  }
}

/// @brief Add thread to scheduler queue.
/// @param t
void RoundRobinScheduler::add_thread(Thread t) {
  RoundRobinScheduler::thread_q.push_back(t);
}
