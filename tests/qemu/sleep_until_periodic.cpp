/**
 * @file sleep_until_periodic.cpp
 * @brief A periodic thread using sleep_until() with an absolute deadline wakes exactly every PERIOD ticks, although it
 *        does a varying amount of work each period: absolute deadlines do not drift the way sleep_for would.
 * @note  The period is at least 10 ms whatever the tick rate, so the work always fits in it. A period the work does not
 *        fit in is an overrun: sleep_until() then returns at once, which is correct but not what this test checks.
 */
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static const uint64_t PERIOD = TIMESLICE_FREQ_HZ >= 300 ? TIMESLICE_FREQ_HZ / 100 : 3;  // >= 10 ms
static const uint32_t PERIODS = 8;

static void periodic() {
  uint64_t start = PreemptFIFOScheduler::tick_count();
  uint64_t deadline = start;
  for (uint32_t k = 1; k <= PERIODS; k++) {
    for (volatile uint32_t work = 0; work < 2000 * k; work++) {
    }
    deadline += PERIOD;
    PreemptFIFOScheduler::sleep_until(deadline);
    yesrtos_test::check(PreemptFIFOScheduler::tick_count() == start + PERIOD * k, "periodic wake-up drifted");
  }
  yesrtos_test::pass();
}

int main() {
  static Thread t(0, periodic);
  PreemptFIFOScheduler::add_thread(&t);
  PreemptFIFOScheduler::start();
  return 0;
}
