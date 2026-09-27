/**
 * @file cas_spinlock.cpp
 * @brief Four equal priority threads increment one counter with atomic_compare_and_swap() and another with a
 *        non-atomic read-modify-write under a spinlock.
 * @note  Each CAS increment yields between reading the counter and the CAS, so the other threads change the value in
 *        between and the mismatch path of the CAS is exercised on every iteration. Nothing yields while holding the
 *        spinlock: under SCHED_FIFO an equal priority spinner would never let the holder run again (CS-006).
 */
#include "preempt_fifo_scheduler.hpp"
#include "spinlock.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static const uint32_t ITERATIONS = 5000;
static const uint32_t THREADS = 4;

static spinlock lock;
static volatile uint32_t cas_counter = 0;
static volatile uint32_t lock_counter = 0;
static volatile uint32_t finished = 0;

static void cas_increment(volatile uint32_t* p) {
  uint32_t v;
  do {
    v = *p;
    PreemptFIFOScheduler::yield();  // let the others update *p before this CAS
  } while (!atomic_compare_and_swap(p, v, v + 1));
}

static void worker() {
  for (uint32_t i = 0; i < ITERATIONS; i++) {
    cas_increment(&cas_counter);

    lock.lock();
    uint32_t tmp = lock_counter;
    for (volatile int delay = 0; delay < 20; delay++) {
    }
    lock_counter = tmp + 1;
    lock.unlock();
  }

  cas_increment(&finished);
  if (finished == THREADS) {
    yesrtos_test::print("cas=");
    yesrtos_test::print_u32(cas_counter);
    yesrtos_test::print(" lock=");
    yesrtos_test::print_u32(lock_counter);
    yesrtos_test::print("\n");
    yesrtos_test::check(cas_counter == THREADS * ITERATIONS, "CAS counter lost an update");
    yesrtos_test::check(lock_counter == THREADS * ITERATIONS, "spinlock counter lost an update");
    yesrtos_test::check(!atomic_compare_and_swap(&cas_counter, 0, 1), "CAS succeeded on a mismatching value");
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
