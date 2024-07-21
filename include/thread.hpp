/**
 * @file thread.hpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief YesRTOS thread header.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#pragma once

#include <armv7m.h>
#include <baremetal_api.h>

namespace YesRTOS {

typedef enum thread_state {
  ACTIVE,
  RUNNING,
  BLOCKED,
  SLEEP,
  COMPLETE
} thread_state_t;

typedef struct thread_info {
  uint32_t id;
  thread_state_t state;
  void (*routine_ptr)(void);
} thread_info_t;

class Thread {
  public:
  Thread(uint32_t id, void (*routine_ptr)(void));
  ~Thread();

  public:
  void init_stack();
  const thread_state_t& get_state() const;
  void set_routine(void (*routine_ptr)(void));
  void set_state(thread_state_t cfg);
  void wake_up();
  void to_sleep();
  void run();

  public:
  // Allocate stack for execution of thread routine, and for saving runtime context when scheduling switching tasks.
  uint32_t allocated_stack[STACK_ALLOCATION_SIZE];
  // Stack pointer pointing to top of the stack.
  uint32_t* stkptr;

  private:
  thread_info_t thread_info;
};
}  // namespace YesRTOS