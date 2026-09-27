/**
 * @file idle_when_all_blocked.cpp
 * @brief Two threads deadlock on two mutexes, so no user thread is ready. The scheduler must fall back to the idle
 *        thread instead of indexing its ready lists with an empty priority bitmap.
 */
#include "mutex.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static Mutex m1, m2;
static volatile bool a_holds_m1 = false;
static volatile bool b_holds_m2 = false;
static volatile uint32_t blocked_threads = 0;

// Called by the idle thread on every pass through its loop.
extern "C" void yesrtos_idle_hook(void) {
  if (blocked_threads == 2) {
    yesrtos_test::pass();
  }
}

// Each thread takes its first mutex, then waits until the other one holds its own before asking for the second, so
// the deadlock does not depend on the order they start in.
static void thread_a() {
  m1.lock();
  a_holds_m1 = true;
  while (!b_holds_m2) PreemptFIFOScheduler::yield();
  blocked_threads = blocked_threads + 1;
  m2.lock();  // blocks forever
  yesrtos_test::fail("thread_a got both mutexes");
}

static void thread_b() {
  m2.lock();
  b_holds_m2 = true;
  while (!a_holds_m1) PreemptFIFOScheduler::yield();
  blocked_threads = blocked_threads + 1;
  m1.lock();  // blocks forever
  yesrtos_test::fail("thread_b got both mutexes");
}

int main() {
  static Thread a(0, thread_a), b(1, thread_b);
  PreemptFIFOScheduler::add_thread(&b);
  PreemptFIFOScheduler::add_thread(&a);
  PreemptFIFOScheduler::start();
  return 0;
}
