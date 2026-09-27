/**
 * @file thread_exit.cpp
 * @brief Threads whose routine returns must end cleanly: marked COMPLETE, never scheduled again, while the remaining
 *        thread keeps running (and is the one marked RUNNING).
 */
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static const uint32_t WORKERS = 3;
static volatile uint32_t runs[WORKERS];

static void worker() {
  uint32_t id = PreemptFIFOScheduler::p_active_thread->thread_info.id;
  runs[id] = runs[id] + 1;
  PreemptFIFOScheduler::yield();  // let the others start before returning
}  // returns: the thread must end here

static Thread w0(0, worker), w1(1, worker), w2(2, worker);

static void observer() {
  while (w0.get_state() != COMPLETE || w1.get_state() != COMPLETE || w2.get_state() != COMPLETE) {
    PreemptFIFOScheduler::yield();
  }
  // Give a wrongly rescheduled worker a chance to run again.
  for (int i = 0; i < 20; i++) PreemptFIFOScheduler::yield();

  for (uint32_t i = 0; i < WORKERS; i++) {
    yesrtos_test::check(runs[i] == 1, "a worker ran more than once or never");
  }
  yesrtos_test::check(PreemptFIFOScheduler::p_active_thread->get_state() == RUNNING, "running thread not marked RUNNING");
  yesrtos_test::pass();
}

int main() {
  static Thread obs(9, observer);
  PreemptFIFOScheduler::add_thread(&obs);
  PreemptFIFOScheduler::add_thread(&w0);
  PreemptFIFOScheduler::add_thread(&w1);
  PreemptFIFOScheduler::add_thread(&w2);
  PreemptFIFOScheduler::start();
  return 0;
}
