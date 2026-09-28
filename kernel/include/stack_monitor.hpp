/**
 * @file stack_monitor.hpp
 * @brief Live view of the thread stacks: draws one bar per thread (current usage, high-water mark, free space) on the
 *        trace output with ANSI escape codes, redrawn in place, so a terminal shows it updating in real time.
 * @note  Add a thread running StackMonitor::routine() at the lowest user priority (MAX_PRIO_LEVEL - 1): it only takes the
 *        CPU when the other threads are sleeping or blocked, and never delays them. Nothing else should print on the
 *        trace output meanwhile, or the frames scroll away.
 * @note  Threads must not be destroyed while the monitor runs.
 */
#pragma once

#include <cstdint>

#ifndef STACK_MONITOR_PERIOD_MS
#define STACK_MONITOR_PERIOD_MS (100U)  // Time between two frames.
#endif

namespace YesRTOS {

class StackMonitor final {
  public:
  /**
   * @brief Most threads shown; further ones are counted in the header but not drawn.
   */
  static constexpr uint32_t MAX_THREADS = 16;

  /**
   * @brief Width of a bar in characters.
   */
  static constexpr uint32_t BAR_WIDTH = 40;

  /**
   * @brief Draw one frame at the top left of the terminal, over the previous one.
   */
  static void draw();

  /**
   * @brief Thread routine: clear the terminal, then draw a frame every STACK_MONITOR_PERIOD_MS.
   */
  [[noreturn]] static void routine();

  private:
  StackMonitor() = delete;
  ~StackMonitor() = delete;
};

}  // namespace YesRTOS
