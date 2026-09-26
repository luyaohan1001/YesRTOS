/**
 * @file exception_priority.cpp
 * @brief PendSV and SysTick must run at the lowest exception priority once the scheduler has started, so a context
 *        switch never preempts an interrupt handler.
 */
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

#define SHPR1 (*((volatile uint8_t*)0xe000ed18))   // System Handler Priority Register 1, byte 0: MemManage
#define SHPR3 (*((volatile uint32_t*)0xe000ed20))  // System Handler Priority Register 3

// Lowest priority value this core implements: unimplemented low bits of a priority field read as zero. Probed on
// another system handler priority field (same width as PendSV's), then restored. BASEPRI is not used for this:
// QEMU does not mask its unimplemented bits.
static uint32_t lowest_priority() {
  uint8_t saved = SHPR1;
  SHPR1 = 0xff;
  uint32_t implemented = SHPR1;
  SHPR1 = saved;
  return implemented;
}

static void worker() {
  uint32_t pendsv = (SHPR3 >> 16) & 0xff;
  uint32_t systick = (SHPR3 >> 24) & 0xff;
  yesrtos_test::print("PendSV=");
  yesrtos_test::print_u32(pendsv);
  yesrtos_test::print(" SysTick=");
  yesrtos_test::print_u32(systick);
  yesrtos_test::print(" lowest=");
  yesrtos_test::print_u32(lowest_priority());
  yesrtos_test::print("\n");
  yesrtos_test::check(pendsv == lowest_priority(), "PendSV is not at the lowest priority");
  yesrtos_test::check(systick == lowest_priority(), "SysTick is not at the lowest priority");
  yesrtos_test::pass();
}

int main() {
  static Thread t0(0, worker);
  PreemptFIFOScheduler::add_thread(&t0);
  PreemptFIFOScheduler::start();
  return 0;
}
