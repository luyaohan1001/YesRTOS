/**
 * @file thread.hpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS thread header declaration.
 * @version 1.0
 * @date 2024-07-12
 * @note This class functions like a task control block that encapsulates all run-time context of a single thread on YesRTOS.
 * @copyright Copyright (c) 2024
 */

#pragma once

#if defined(ARMV7M)
  #include <armv7m.h>
  #include <baremetal_api.h>
#else
  static_assert(0, "ARCH not defined");
#endif

#include <cstdint>


#if defined(HOST_PLATFORM)
#include <iostream>
#endif

namespace YesRTOS {

// forward declaration
class Thread;

typedef enum thread_state {
  READY,
  RUNNING,
  BLOCKED,
  SLEEP,
  COMPLETE
} thread_state_t;

typedef struct thread_info {
  thread_state_t state;
  void (*routine_ptr)(void);
  Thread* p_next;
  Thread* p_prev;
  uint32_t id;
  uint8_t priority;
  uint64_t wake_tick;  // tick count at which a SLEEP thread becomes ready again
} thread_info_t;

class Thread {
  public:
  /**
   * @brief Construct a new Thread object
   * @param id Unique ID for the thread, defined by the user.
   * @param routine_ptr Function pointer to the execution routine.
   * @param priority Thread priority, by default 0 (highest priority)
   */
  Thread(uint32_t id, void (*routine_ptr)(void), uint8_t priority = 0);
  ~Thread();

  public:
  void init_stack();
  const thread_state_t& get_state() const;
  void set_routine(void (*routine_ptr)(void));
  void set_state(thread_state_t cfg);
  void wake_up();
  void to_sleep();
  void run();
  bool operator==(const Thread& other) const;

  /**
   * @brief Size of the thread stack in bytes.
   */
  uint32_t stack_size_bytes() const;

  /**
   * @brief Bytes of the stack in use right now: up to the saved stack pointer for a thread that is switched out, up to
   *        PSP for the running thread. Includes the saved context (r0-r3, r12, lr, pc, xpsr, r4-r11).
   */
  uint32_t stack_used_bytes() const;

  /**
   * @brief High-water mark: the most stack bytes ever used, found by scanning from the far end for words that still
   *        hold STACK_PAINT, and never less than stack_used_bytes(). Equal to stack_size_bytes() when the stack has
   *        overflowed (or used every word).
   * @note  Takes time linear in the unused part of the stack; meant for monitoring, not for hot paths.
   */
  uint32_t stack_peak_bytes() const;

  /**
   * @brief Pattern every stack word is filled with at construction; a word that still holds it was never written.
   */
  static constexpr uint32_t STACK_PAINT = 0xA5A5A5A5UL;

  /**
   * @brief Every constructed Thread, newest first (linked through p_registry_next), so monitors can find all threads,
   *        including blocked ones whose lists belong to a Mutex or Semaphore.
   */
  static Thread* registry_head;
  Thread* p_registry_next;

  public:
  // Allocate stack for execution of thread routine, and for saving runtime context when scheduling switching tasks.
  // AAPCS requires SP to be 8-byte aligned at every public function entry, and the thread starts with SP at the end of
  // this array; uint32_t alone only guarantees 4.
  static_assert((STACK_ALLOCATION_SIZE * sizeof(uint32_t)) % 8 == 0, "thread stack size must keep the stack top 8-byte aligned");
  alignas(8) uint32_t allocated_stack[STACK_ALLOCATION_SIZE];

  // Stack pointer pointing to top of the stack.
  volatile uint32_t* stkptr;
  thread_info_t thread_info;

  public:
#if defined(HOST_PLATFORM)
  friend std::ostream& operator<<(std::ostream& os, const Thread& t) {
    os << "thread id: " << t.thread_info.id;
    return os;  // Return the ostream to allow chaining of <<
  }
#endif
};
}  // namespace YesRTOS
