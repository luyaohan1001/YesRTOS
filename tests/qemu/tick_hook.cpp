/**
 * @file tick_hook.cpp
 * @brief yesrtos_tick_hook() runs from the SysTick interrupt on every tick: here it releases a semaphore a thread waits
 *        on, which must wake up once per tick.
 */
#include "preempt_fifo_scheduler.hpp"
#include "semaphore.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static Semaphore tick(0, 1);
static volatile uint32_t hook_calls = 0;

extern "C" void yesrtos_tick_hook(void) {
  hook_calls = hook_calls + 1;
  tick.release();
}

static void waiter() {
  for (int i = 0; i < 5; i++) {
    tick.acquire();  // blocks until the next tick
  }
  yesrtos_test::print("hook calls=");
  yesrtos_test::print_u32(hook_calls);
  yesrtos_test::print("\n");
  yesrtos_test::check(hook_calls >= 5, "woke up more often than the tick hook ran");
  yesrtos_test::pass();
}

int main() {
  static Thread t(0, waiter);
  PreemptFIFOScheduler::add_thread(&t);
  PreemptFIFOScheduler::start();
  return 0;
}
