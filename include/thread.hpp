#pragma once

#include <cstdint>

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
  bool yield_request;
  void (*routine_ptr)(Thread& self);
} thread_info_t;

class Thread {
 private:
  thread_info_t thread_info;

 public:
  Thread(thread_info_t cfg);
  ~Thread();

 public:
  const thread_state_t& get_state() const;
  void set_rountine(void (*routine_ptr)(Thread& thread_handle));
  void run();
  void yield();
  void resume();
};
}  // namespace YesRTOS