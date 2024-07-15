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

namespace YesRTOS {

typedef enum thread_state {
  ACTIVE,
  RUNNING,
  BLOCKED,
  SLEEP,
  COMPLETE
} thread_state_t;

// Forward declaration.
class Thread;

typedef struct thread_info {
  uint32_t id;
  thread_state_t state;
  void (*routine_ptr)(Thread* self);
} thread_info_t;

class Thread {
 public:
  Thread(uint32_t id, void (*routine_ptr)(Thread* self));
  ~Thread();

 public:
  const thread_state_t& get_state() const;
  void set_rountine(void (*routine_ptr)(Thread* thread_handle));
  void set_state(thread_state_t cfg);
  void wake_up();
  void to_sleep();
  uint32_t* get_thread_stack_ptr();
  void run();

 public:
  uint32_t thread_stack_ptr[CONTEXT_SAVE_STACK_SIZE];

 private:
  thread_info_t thread_info;
};
}  // namespace YesRTOS