/**
 * @file mutex_exclusion.cpp
 * @brief Four threads increment a shared counter with a non-atomic read-modify-write under a YesRTOS::Mutex.
 * @note  The owner yields inside the critical section, so every iteration forces the other threads to contend for a
 *        taken mutex and to be woken by unlock(). Equal priority threads are not time-sliced (SCHED_FIFO), so without
 *        the yield they would simply run one after another.
 */
#include "mutex.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static const uint32_t ITERATIONS = 2000;
static const uint32_t THREADS = 4;

static Mutex lock;
static volatile uint32_t counter = 0;
static volatile uint32_t inside = 0;
static volatile uint32_t finished = 0;

static void worker() {
  for (uint32_t i = 0; i < ITERATIONS; i++) {
    lock.lock();
    yesrtos_test::check(++inside == 1, "two threads inside the critical section");
    uint32_t tmp = counter;
    PreemptFIFOScheduler::yield();  // let the other threads run into the taken mutex
    yesrtos_test::check(inside == 1, "another thread entered the critical section");
    counter = tmp + 1;
    inside = inside - 1;
    lock.unlock();
  }

  lock.lock();
  bool last = (++finished == THREADS);
  lock.unlock();
  if (last) {
    yesrtos_test::print("counter=");
    yesrtos_test::print_u32(counter);
    yesrtos_test::print("\n");
    yesrtos_test::check(counter == THREADS * ITERATIONS, "lost update");
    yesrtos_test::pass();
  }
}

int main() {
  static Thread t0(0, worker), t1(1, worker), t2(2, worker), t3(3, worker);
  PreemptFIFOScheduler::add_thread(&t0);
  PreemptFIFOScheduler::add_thread(&t1);
  PreemptFIFOScheduler::add_thread(&t2);
  PreemptFIFOScheduler::add_thread(&t3);
  PreemptFIFOScheduler::start();
  return 0;
}
