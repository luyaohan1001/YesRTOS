/**
 * @file startup_pending_switch.cpp
 * @brief A context switch requested before the scheduler runs its first thread must be harmless. This is what happens
 *        when SysTick fires between systick_clk_init() and the SVC that starts the first thread (seen under host load
 *        with a fast tick): PendSV then runs from main(), where PSP does not point at any thread stack yet.
 */
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static void worker() {
  yesrtos_test::pass();
}

int main() {
  static Thread t(0, worker);
  PreemptFIFOScheduler::add_thread(&t);
  request_context_switch();  // PendSV before start(): the startup window made deterministic
  PreemptFIFOScheduler::start();
  return 0;
}
