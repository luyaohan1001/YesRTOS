/**
 * @file mutex_states.cpp
 * @brief Walks a Mutex through its states: uncontended lock (LOCKED), a waiter arriving (LOCKED_CONTENDED), hand-over
 *        to the waiter on unlock (LOCKED again, owned by the waiter), and release (UNLOCKED).
 */
#include "mutex.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static Mutex m;
static volatile bool waiter_started = false;
static volatile bool waiter_owns = false;

static void waiter() {
  waiter_started = true;
  m.lock();  // blocks until first() hands the mutex over
  waiter_owns = true;
  yesrtos_test::check(m.owner == PreemptFIFOScheduler::p_active_thread, "woken waiter is not the owner");
  yesrtos_test::check(m.locked == Mutex::LOCKED, "no waiter left, but mutex still marked contended");
  m.unlock();
  yesrtos_test::check(m.locked == Mutex::UNLOCKED && m.owner == nullptr, "mutex not released");
  yesrtos_test::pass();
}

static Thread waiter_thread(1, waiter);

static void first() {
  m.lock();
  yesrtos_test::check(m.locked == Mutex::LOCKED, "uncontended lock did not mark LOCKED");
  PreemptFIFOScheduler::add_thread(&waiter_thread);
  while (!waiter_started || waiter_thread.get_state() != BLOCKED) PreemptFIFOScheduler::yield();
  yesrtos_test::check(m.locked == Mutex::LOCKED_CONTENDED, "waiter blocked, but mutex not marked contended");
  m.unlock();  // hands over to the waiter; returning lets it run and finish the test
}

int main() {
  static Thread first_thread(0, first);
  PreemptFIFOScheduler::add_thread(&first_thread);
  PreemptFIFOScheduler::start();
  return 0;
}
