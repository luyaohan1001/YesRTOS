/**
 * @file stack_usage.cpp
 * @brief Stack usage queries and the thread registry:
 *        - a new thread uses exactly its initial context frame (16 words), and so is its high-water mark;
 *        - every constructed thread is in Thread::registry_head, and a destroyed one is removed;
 *        - a thread switched out inside a deep call shows that depth in stack_used_bytes(); once it returns, the
 *          current use drops but stack_peak_bytes() keeps the depth;
 *        - the running thread's current use comes from PSP;
 *        - StackMonitor::draw() walks all threads and returns.
 */
#include "preempt_fifo_scheduler.hpp"
#include "stack_monitor.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static const uint32_t DEEP_WORDS = 200;  // 800 bytes of locals in deep_call()
static const uint32_t INITIAL_FRAME_BYTES = 16 * sizeof(uint32_t);

static volatile uint32_t phase = 0;

static bool registered(const Thread* t) {
  for (Thread* p = Thread::registry_head; p; p = p->p_registry_next) {
    if (p == t) return true;
  }
  return false;
}

static void deep_call() {
  volatile uint32_t locals[DEEP_WORDS];
  for (uint32_t i = 0; i < DEEP_WORDS; i++) locals[i] = i;
  phase = 1;
  PreemptFIFOScheduler::sleep_for_ticks(3);  // switched out here, with the locals on the stack
  (void)locals[0];
}

static void worker() {
  deep_call();
  phase = 2;
  while (1) {
    PreemptFIFOScheduler::sleep_for_ticks(1000);
  }
}

static Thread worker_thread(1, worker, 0);

static void checker() {
  // The worker (higher priority) ran first and now sleeps inside deep_call().
  yesrtos_test::check(phase == 1, "worker did not reach deep_call()");
  uint32_t deep_used = worker_thread.stack_used_bytes();
  yesrtos_test::print("deep used=");
  yesrtos_test::print_u32(deep_used);
  yesrtos_test::print(" peak=");
  yesrtos_test::print_u32(worker_thread.stack_peak_bytes());
  yesrtos_test::print("\n");
  yesrtos_test::check(deep_used >= DEEP_WORDS * sizeof(uint32_t), "used bytes miss the locals of the deep call");
  yesrtos_test::check(deep_used < worker_thread.stack_size_bytes(), "used bytes not below the stack size");
  yesrtos_test::check(worker_thread.stack_peak_bytes() >= deep_used, "peak below the current use");

  // Wait until the worker returned from deep_call() and sleeps shallow.
  while (phase != 2) PreemptFIFOScheduler::sleep_for_ticks(1);
  uint32_t shallow_used = worker_thread.stack_used_bytes();
  yesrtos_test::print("shallow used=");
  yesrtos_test::print_u32(shallow_used);
  yesrtos_test::print(" peak=");
  yesrtos_test::print_u32(worker_thread.stack_peak_bytes());
  yesrtos_test::print("\n");
  yesrtos_test::check(shallow_used < DEEP_WORDS * sizeof(uint32_t), "used bytes did not drop after the deep call");
  yesrtos_test::check(worker_thread.stack_peak_bytes() >= deep_used, "peak forgot the deep call");

  // The running thread: PSP, not the stale saved stack pointer.
  Thread* self = PreemptFIFOScheduler::p_active_thread;
  uint32_t self_used = self->stack_used_bytes();
  yesrtos_test::check(self_used > 0 && self_used < self->stack_size_bytes(), "running thread use out of range");
  volatile uint32_t marker;
  uint32_t top = (uint32_t)(uintptr_t)&self->allocated_stack[STACK_ALLOCATION_SIZE];
  uint32_t local_depth = top - (uint32_t)(uintptr_t)&marker;
  yesrtos_test::check(self_used + 64 >= local_depth && self_used <= local_depth + 64,
                      "running thread use does not match its stack pointer");

  StackMonitor::draw();
  yesrtos_test::print("\x1b[0m\n");
  yesrtos_test::pass();
}

static Thread checker_thread(2, checker, 1);

int main() {
  yesrtos_test::check(worker_thread.stack_used_bytes() == INITIAL_FRAME_BYTES, "new thread does not use its initial frame");
  yesrtos_test::check(worker_thread.stack_peak_bytes() == INITIAL_FRAME_BYTES, "new thread peak is not its initial frame");
  yesrtos_test::check(worker_thread.stack_size_bytes() == STACK_ALLOCATION_SIZE * sizeof(uint32_t), "wrong stack size");
  yesrtos_test::check(registered(&worker_thread) && registered(&checker_thread), "thread missing from the registry");
  {
    Thread temporary(9, worker, 0);
    yesrtos_test::check(registered(&temporary), "new thread missing from the registry");
  }
  uint32_t count = 0;
  for (Thread* p = Thread::registry_head; p; p = p->p_registry_next) count++;
  yesrtos_test::check(count == 3, "registry is not worker, checker and idle after destroying a thread");

  PreemptFIFOScheduler::add_thread(&worker_thread);
  PreemptFIFOScheduler::add_thread(&checker_thread);
  PreemptFIFOScheduler::start();
  return 0;
}
