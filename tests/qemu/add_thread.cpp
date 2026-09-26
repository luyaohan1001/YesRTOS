/**
 * @file add_thread.cpp
 * @brief add_thread() must reject priorities outside the user range (including the idle level) and null threads
 *        without touching the ready lists, and must be usable from a running thread.
 */
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static volatile bool late_thread_ran = false;

static void late_thread() {
  late_thread_ran = true;
}

static void first_thread() {
  static Thread idle_level(10, late_thread, MAX_PRIO_LEVEL);  // reserved for the idle thread
  static Thread out_of_range(11, late_thread, 200);
  Thread* heads_before[MAX_PRIO_LEVEL + 1];
  for (uint32_t i = 0; i <= MAX_PRIO_LEVEL; i++) heads_before[i] = PreemptFIFOScheduler::ready_list_heads[i];
  uint32_t bitmap_before = PreemptFIFOScheduler::prio_bitmap;

  yesrtos_test::check(!PreemptFIFOScheduler::add_thread(&idle_level), "accepted the idle priority level");
  yesrtos_test::check(!PreemptFIFOScheduler::add_thread(&out_of_range), "accepted priority 200");
  yesrtos_test::check(!PreemptFIFOScheduler::add_thread(nullptr), "accepted a null thread");
  for (uint32_t i = 0; i <= MAX_PRIO_LEVEL; i++) {
    yesrtos_test::check(PreemptFIFOScheduler::ready_list_heads[i] == heads_before[i], "rejected thread changed a ready list");
  }
  yesrtos_test::check(PreemptFIFOScheduler::prio_bitmap == bitmap_before, "rejected thread changed prio_bitmap");

  // Adding a thread after start(), from a thread.
  static Thread late(12, late_thread, 0);
  yesrtos_test::check(PreemptFIFOScheduler::add_thread(&late), "rejected a valid thread");
  for (int i = 0; i < 20 && !late_thread_ran; i++) request_context_switch();
  yesrtos_test::check(late_thread_ran, "thread added at run time never ran");
  yesrtos_test::pass();
}

int main() {
  static Thread first(0, first_thread, 0);
  yesrtos_test::check(PreemptFIFOScheduler::add_thread(&first), "rejected a valid thread before start");
  PreemptFIFOScheduler::start();
  return 0;
}
