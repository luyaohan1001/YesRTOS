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
#elif defined(RV32I)
  #include <rv32i.h>
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

  public:
  // Allocate stack for execution of thread routine, and for saving runtime context when scheduling switching tasks.
  uint32_t allocated_stack[STACK_ALLOCATION_SIZE];

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
