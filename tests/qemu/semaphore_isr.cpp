/**
 * @file semaphore_isr.cpp
 * @brief An interrupt handler releases a semaphore a higher priority thread waits on: the waiter must run as soon as
 *        the handler returns, before the interrupted lower priority thread continues.
 * @note  IRQ 0 is triggered by software through the NVIC (vector entry IRQ0_Handler in the shared startup
 *        file), so the test does not depend on any board peripheral.
 */
#include "preempt_fifo_scheduler.hpp"
#include "semaphore.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

#define NVIC_ISER0 (*((volatile uint32_t*)0xe000e100))  // Interrupt Set-Enable Register 0
#define NVIC_ISPR0 (*((volatile uint32_t*)0xe000e200))  // Interrupt Set-Pending Register 0

static Semaphore sem(0);
static char trace[8];
static volatile uint32_t trace_len = 0;

static void log(char event) {
  trace[trace_len] = event;
  trace_len = trace_len + 1;
}

extern "C" void IRQ0_Handler(void) {
  yesrtos_test::check(sem.try_acquire() == false, "try_acquire in the handler found a token");
  yesrtos_test::check(sem.release(), "release from the handler failed");
}

static void waiter() {
  log('w');
  sem.acquire();  // blocks; released from the interrupt handler
  log('W');
}

static void interrupted() {
  log('l');
  NVIC_ISER0 = 1u;  // enable IRQ 0
  NVIC_ISPR0 = 1u;  // and make it pending: the handler runs right away
  log('L');

  trace[trace_len] = '\0';
  yesrtos_test::print("trace=");
  yesrtos_test::print(trace);
  yesrtos_test::print(" expected=wlWL\n");
  const char* expected = "wlWL";
  for (uint32_t i = 0; expected[i] != '\0' || i < trace_len; i++) {
    yesrtos_test::check(trace[i] == expected[i], "waiter did not preempt right after the interrupt released it");
  }
  yesrtos_test::pass();
}

int main() {
  static Thread w(0, waiter, 0), l(1, interrupted, 1);
  PreemptFIFOScheduler::add_thread(&w);
  PreemptFIFOScheduler::add_thread(&l);
  PreemptFIFOScheduler::start();
  return 0;
}
