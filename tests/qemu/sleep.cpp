/**
 * @file sleep.cpp
 * @brief Tick counter and sleep:
 *        - sleep_for_ticks(5) returns exactly 5 ticks later, and the CPU goes to other threads meanwhile (a lower
 *          priority thread and the idle thread both run while the sleeper waits);
 *        - sleep_for_ms() sleeps at least the requested time: rounded up to whole ticks, plus one for the partial tick
 *          the call starts in;
 *        - sleep_for_ticks(0) returns at once;
 *        - threads sleeping until different ticks wake in deadline order, equal deadlines in the order they slept.
 */
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static volatile uint32_t low_priority_runs = 0;
static volatile uint32_t idle_runs = 0;
static volatile uint64_t base_tick = 0;
static char order[8];
static volatile uint32_t order_len = 0;

extern "C" void yesrtos_idle_hook(void) {
  idle_runs = idle_runs + 1;
}

static void sleeper(char name, uint64_t offset) {
  PreemptFIFOScheduler::sleep_until(base_tick + offset);
  order[order_len] = name;
  order_len = order_len + 1;
}
static void sleeper_a() { sleeper('A', 3); }
static void sleeper_b() { sleeper('B', 1); }
static void sleeper_c() { sleeper('C', 2); }
static void sleeper_d() { sleeper('D', 2); }  // same deadline as C, slept after it

static void main_thread() {
  // Exact tick count, and the CPU is handed to others while sleeping.
  uint64_t t0 = PreemptFIFOScheduler::tick_count();
  PreemptFIFOScheduler::sleep_for_ticks(5);
  uint64_t t1 = PreemptFIFOScheduler::tick_count();
  yesrtos_test::check(t1 - t0 == 5, "sleep_for_ticks(5) did not return 5 ticks later");
  yesrtos_test::check(low_priority_runs > 0, "a lower priority thread did not run while sleeping");
  yesrtos_test::check(idle_runs > 0, "the idle thread did not run while sleeping");

  // At least the requested time.
  const uint32_t ms = 25;
  const uint64_t tick_us = 1000000 / TIMESLICE_FREQ_HZ;  // tick period in microseconds
  t0 = PreemptFIFOScheduler::tick_count();
  PreemptFIFOScheduler::sleep_for_ms(ms);
  t1 = PreemptFIFOScheduler::tick_count();
  yesrtos_test::check((t1 - t0 - 1) * tick_us >= ms * 1000, "sleep_for_ms() slept less than requested");

  t0 = PreemptFIFOScheduler::tick_count();
  PreemptFIFOScheduler::sleep_for_ticks(0);
  yesrtos_test::check(PreemptFIFOScheduler::tick_count() - t0 <= 1, "sleep_for_ticks(0) did not return at once");

  // Wake-up order.
  base_tick = PreemptFIFOScheduler::tick_count() + 2;
  static Thread a(1, sleeper_a, 0), b(2, sleeper_b, 0), c(3, sleeper_c, 0), d(4, sleeper_d, 0);
  PreemptFIFOScheduler::add_thread(&a);
  PreemptFIFOScheduler::add_thread(&b);
  PreemptFIFOScheduler::add_thread(&c);
  PreemptFIFOScheduler::add_thread(&d);
  PreemptFIFOScheduler::sleep_until(base_tick + 5);
  order[order_len] = '\0';
  yesrtos_test::print("wake order=");
  yesrtos_test::print(order);
  yesrtos_test::print(" expected=BCDA\n");
  const char* expected = "BCDA";
  for (uint32_t i = 0; expected[i] != '\0' || i < order_len; i++) {
    yesrtos_test::check(order[i] == expected[i], "threads did not wake in deadline order");
  }
  yesrtos_test::pass();
}

static void low_priority() {
  while (1) {
    low_priority_runs = low_priority_runs + 1;
    PreemptFIFOScheduler::sleep_for_ticks(1);  // let the idle thread run too
  }
}

int main() {
  static Thread m(0, main_thread, 0), l(9, low_priority, 1);
  PreemptFIFOScheduler::add_thread(&m);
  PreemptFIFOScheduler::add_thread(&l);
  PreemptFIFOScheduler::start();
  return 0;
}
