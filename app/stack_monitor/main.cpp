/**
 * @file main.cpp
 * @brief Stack monitor demo: each worker recurses to a depth that follows its own triangle wave and sleeps at the
 *        bottom, so its stack use rises and falls; StackMonitor draws all stacks as live bars on the terminal.
 * @note  Run with: cmake --build --preset qemu --target qemu-stack-monitor (Ctrl-A X to quit).
 */
#include "preempt_fifo_scheduler.hpp"
#include "stack_monitor.hpp"

using namespace YesRTOS;

static const uint32_t FRAME_WORDS = 16;  // stack taken by each level of recursion, besides the call overhead

/**
 * @brief Take `depth` more frames of stack, then sleep at the bottom: the monitor sees the thread at its deepest.
 */
static void descend(uint32_t depth, uint32_t sleep_ms) {
  volatile uint32_t frame[FRAME_WORDS];
  frame[0] = depth;
  if (depth > 0) {
    descend(depth - 1, sleep_ms);
  } else {
    PreemptFIFOScheduler::sleep_for_ms(sleep_ms);
  }
  (void)frame[0];
}

/**
 * @brief Worker routine: recursion depth goes 0, 1, ..., MAX_DEPTH, ..., 1, 0, ... one step every STEP_MS.
 */
template <uint32_t MAX_DEPTH, uint32_t STEP_MS>
static void worker() {
  uint32_t depth = 0;
  bool deeper = true;
  while (1) {
    descend(depth, STEP_MS);
    if (deeper && depth == MAX_DEPTH) deeper = false;
    if (!deeper && depth == 0) deeper = true;
    depth = deeper ? depth + 1 : depth - 1;
  }
}

/**
 * @brief Worker that went deep once at start-up and stays shallow: its peak stays high while its current use is low.
 */
static void burst_once() {
  descend(10, 1);
  while (1) {
    PreemptFIFOScheduler::sleep_for_ms(1000);
  }
}

int main() {
  static Thread shallow(1, worker<3, 150>, 0);
  static Thread medium(2, worker<7, 80>, 1);
  static Thread deep(3, worker<13, 60>, 1);
  static Thread burst(4, burst_once, 2);
  static Thread monitor(5, StackMonitor::routine, MAX_PRIO_LEVEL - 1);

  PreemptFIFOScheduler::add_thread(&shallow);
  PreemptFIFOScheduler::add_thread(&medium);
  PreemptFIFOScheduler::add_thread(&deep);
  PreemptFIFOScheduler::add_thread(&burst);
  PreemptFIFOScheduler::add_thread(&monitor);
  PreemptFIFOScheduler::start();
  return 0;
}
