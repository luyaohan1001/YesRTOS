/**
 * @file semaphore.cpp
 * @brief Counting semantics (bounded count, try_acquire), then two threads blocking on an empty semaphore: release()
 *        hands each token straight to the longest waiter (the count stays 0) and wakes them in the order they blocked.
 */
#include "preempt_fifo_scheduler.hpp"
#include "semaphore.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static Semaphore sem(0);
static char trace[8];
static volatile uint32_t trace_len = 0;

static void log(char event) {
  trace[trace_len] = event;
  trace_len = trace_len + 1;
}

static void waiter_a() {
  log('a');
  sem.acquire();
  log('A');
}

static void waiter_b() {
  log('b');
  sem.acquire();
  log('B');
}

static Thread a(0, waiter_a, 1), b(1, waiter_b, 1);

static void producer() {
  // Counting: initial 2, at most 3.
  static Semaphore counted(2, 3);
  yesrtos_test::check(counted.try_acquire() && counted.try_acquire(), "could not take the initial count");
  yesrtos_test::check(!counted.try_acquire(), "try_acquire succeeded on an empty semaphore");
  yesrtos_test::check(counted.release() && counted.release() && counted.release(), "release below the maximum failed");
  yesrtos_test::check(!counted.release() && counted.count() == 3, "count went past its maximum");
  counted.acquire();  // available: must not block
  yesrtos_test::check(counted.count() == 2, "acquire did not take one unit");

  // Blocking: both waiters ran first and blocked on the empty semaphore.
  yesrtos_test::check(a.get_state() == BLOCKED && b.get_state() == BLOCKED, "waiters did not block");
  sem.release();
  sem.release();
  yesrtos_test::check(sem.count() == 0, "release() counted up instead of handing the token to a waiter");
  PreemptFIFOScheduler::yield();  // let the woken waiters run

  trace[trace_len] = '\0';
  yesrtos_test::print("trace=");
  yesrtos_test::print(trace);
  yesrtos_test::print(" expected=abAB\n");
  const char* expected = "abAB";
  for (uint32_t i = 0; expected[i] != '\0' || i < trace_len; i++) {
    yesrtos_test::check(trace[i] == expected[i], "waiters not woken in the order they blocked");
  }
  yesrtos_test::pass();
}

int main() {
  static Thread p(2, producer, 1);
  PreemptFIFOScheduler::add_thread(&a);
  PreemptFIFOScheduler::add_thread(&b);
  PreemptFIFOScheduler::add_thread(&p);
  PreemptFIFOScheduler::start();
  return 0;
}
