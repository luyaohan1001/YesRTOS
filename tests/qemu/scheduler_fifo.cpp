/**
 * @file scheduler_fifo.cpp
 * @brief SCHED_FIFO semantics of PreemptFIFOScheduler:
 *        - equal priority threads run in the order they were added,
 *        - a running thread is not time-sliced against equal priorities, only yield()/blocking/exit let them run,
 *        - a higher priority thread made ready preempts at once,
 *        - the preempted thread resumes before the other threads of its priority.
 */
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

#define SYST_CSR (*((volatile uint32_t*)0xe000e010))
#define SYST_CSR_COUNTFLAG (1UL << 16UL)

static char trace[8];
static volatile uint32_t trace_len = 0;

static void log(char event) {
  trace[trace_len] = event;
  trace_len = trace_len + 1;
}

// Busy for at least `ticks` SysTick periods (COUNTFLAG is set on every reload and cleared by reading SYST_CSR).
static void busy_for_ticks(uint32_t ticks) {
  (void)SYST_CSR;
  uint32_t seen = 0;
  while (seen < ticks) {
    if (SYST_CSR & SYST_CSR_COUNTFLAG) seen++;
  }
}

static void high() {
  log('H');
}

static Thread high_thread(9, high, 0);

static void thread_a() {
  log('a');
  busy_for_ticks(3);  // SysTick must not hand the CPU to B or C
  PreemptFIFOScheduler::add_thread(&high_thread);  // higher priority: runs before add_thread() returns here
  log('b');
  PreemptFIFOScheduler::yield();  // now B and C, in the order they were added
  log('c');

  trace[trace_len] = '\0';
  yesrtos_test::print("trace=");
  yesrtos_test::print(trace);
  yesrtos_test::print(" expected=aHbBCc\n");
  const char* expected = "aHbBCc";
  for (uint32_t i = 0; expected[i] != '\0' || i < trace_len; i++) {
    yesrtos_test::check(trace[i] == expected[i], "threads did not run in SCHED_FIFO order");
  }
  yesrtos_test::pass();
}

static void thread_b() {
  log('B');
}

static void thread_c() {
  log('C');
}

int main() {
  static Thread a(0, thread_a, 1), b(1, thread_b, 1), c(2, thread_c, 1);
  PreemptFIFOScheduler::add_thread(&a);
  PreemptFIFOScheduler::add_thread(&b);
  PreemptFIFOScheduler::add_thread(&c);
  PreemptFIFOScheduler::start();
  return 0;
}
