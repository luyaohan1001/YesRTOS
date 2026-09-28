/**
 * @file main.cpp
 * @brief Stack monitor demo: threads doing different kinds of work, so their stacks behave differently, drawn live as
 *        bars by StackMonitor.
 *        - control:   periodic 10 ms loop with sleep_until(), tiny and flat stack
 *        - producer / consumer: ring buffer guarded by two semaphores; the consumer checksums each item recursively,
 *                     deeper for larger items, and is BLOCKED while the buffer is empty
 *        - log_a / log_b: share a mutex and sleep while holding it deep in a call chain, so the other one is BLOCKED
 *        - quicksort: recursive quicksort of a random array on its own stack; the depth depends on the pivots
 *        - fibonacci: naive recursive fib(n) with n cycling through 5..20, deep and CPU heavy
 *        - wave:      recursion depth following a triangle wave, sleeping at the bottom, so its current use rises and
 *                     falls on screen
 *        - danger:    like wave but deeper, reaching the red zone
 *        - burst:     went deep once at start-up, then stays shallow: low current use, high peak
 *        Threads that only compute (quicksort, fibonacci) are always seen at their shallow sleep point, since the
 *        monitor runs at the lowest priority; their peak still shows how deep they went.
 * @note  Run with: cmake --build --preset qemu --target qemu-stack-monitor (Ctrl-A X to quit).
 */
#include "mutex.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "semaphore.hpp"
#include "stack_monitor.hpp"

using namespace YesRTOS;

/**
 * @brief Small pseudo random generator (xorshift32), one state per caller.
 */
static uint32_t next_random(uint32_t& state) {
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return state;
}

// ---------------------------------------------------------------- control

static void control() {
  uint64_t deadline = PreemptFIFOScheduler::tick_count();
  volatile uint32_t setpoint = 0;
  while (1) {
    setpoint = setpoint + 1;
    deadline += 1;  // one tick (10 ms at 100 Hz)
    PreemptFIFOScheduler::sleep_until(deadline);
  }
}

// ---------------------------------------------------------------- producer / consumer

static const uint32_t QUEUE_SIZE = 4;
static uint32_t queue[QUEUE_SIZE];
static uint32_t queue_head = 0;
static uint32_t queue_tail = 0;
static Semaphore free_slots(QUEUE_SIZE);
static Semaphore queued_items(0);

static void producer() {
  uint32_t rng = 0x1234567U;
  while (1) {
    free_slots.acquire();
    queue[queue_tail] = next_random(rng) % 12;  // item value = checksum depth
    queue_tail = (queue_tail + 1) % QUEUE_SIZE;
    queued_items.release();
    PreemptFIFOScheduler::sleep_for_ms(40 + next_random(rng) % 200);
  }
}

static uint32_t checksum(uint32_t depth, uint32_t seed) {
  volatile uint32_t block[12];
  for (uint32_t i = 0; i < 12; i++) block[i] = seed * 31 + i;
  uint32_t sum = block[0] ^ block[11];
  if (depth > 0) {
    sum += checksum(depth - 1, sum);
  } else {
    PreemptFIFOScheduler::sleep_for_ms(30);  // "processing" at the bottom
  }
  return sum;
}

static void consumer() {
  volatile uint32_t total = 0;
  while (1) {
    queued_items.acquire();  // BLOCKED while the queue is empty
    uint32_t item = queue[queue_head];
    queue_head = (queue_head + 1) % QUEUE_SIZE;
    free_slots.release();
    total = total + checksum(item, item);
  }
}

// ---------------------------------------------------------------- log_a / log_b

static Mutex log_lock;

static void format_line(uint32_t depth, uint32_t sleep_ms) {
  volatile char line[48];
  for (uint32_t i = 0; i < sizeof(line); i++) line[i] = static_cast<char>('a' + (depth + i) % 26);
  if (depth > 0) {
    format_line(depth - 1, sleep_ms);
  } else {
    PreemptFIFOScheduler::sleep_for_ms(sleep_ms);  // "writing out" while holding the lock
  }
}

template <uint32_t DEPTH, uint32_t HOLD_MS, uint32_t PAUSE_MS>
static void logger() {
  while (1) {
    log_lock.lock();  // BLOCKED while the other logger holds it
    format_line(DEPTH, HOLD_MS);
    log_lock.unlock();
    PreemptFIFOScheduler::sleep_for_ms(PAUSE_MS);
  }
}

// ---------------------------------------------------------------- quicksort

static void quicksort(uint32_t* a, int32_t lo, int32_t hi) {
  if (lo >= hi) return;
  uint32_t pivot = a[hi];
  int32_t i = lo;
  for (int32_t j = lo; j < hi; j++) {
    if (a[j] < pivot) {
      uint32_t t = a[i];
      a[i] = a[j];
      a[j] = t;
      i++;
    }
  }
  uint32_t t = a[i];
  a[i] = a[hi];
  a[hi] = t;
  quicksort(a, lo, i - 1);
  quicksort(a, i + 1, hi);
}

static const uint32_t SORT_SIZE = 20;  // nearly sorted input recurses about SORT_SIZE deep: keep it within the stack

static void sorter() {
  uint32_t rng = 0xC0FFEEU;
  uint32_t data[SORT_SIZE];
  while (1) {
    // Sometimes nearly sorted input, the worst case for this pivot choice: much deeper recursion.
    bool nearly_sorted = next_random(rng) % 4 == 0;
    for (uint32_t i = 0; i < SORT_SIZE; i++) data[i] = nearly_sorted ? i * 2 + next_random(rng) % 3 : next_random(rng) % 1000;
    quicksort(data, 0, SORT_SIZE - 1);
    PreemptFIFOScheduler::sleep_for_ms(300);
  }
}

// ---------------------------------------------------------------- fibonacci

static uint32_t fib(uint32_t n) {
  return n < 2 ? n : fib(n - 1) + fib(n - 2);
}

static volatile uint32_t fib_result;

static void fibonacci() {
  uint32_t n = 5;
  while (1) {
    fib_result = fib(n);
    n = n == 20 ? 5 : n + 1;
    PreemptFIFOScheduler::sleep_for_ms(250);
  }
}

// ---------------------------------------------------------------- wave / danger / burst

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
 * @brief Recursion depth goes 0, 1, ..., MAX_DEPTH, ..., 1, 0, ... one step every STEP_MS.
 */
template <uint32_t MAX_DEPTH, uint32_t STEP_MS>
static void wave() {
  uint32_t depth = 0;
  bool deeper = true;
  while (1) {
    descend(depth, STEP_MS);
    if (deeper && depth == MAX_DEPTH) deeper = false;
    if (!deeper && depth == 0) deeper = true;
    depth = deeper ? depth + 1 : depth - 1;
  }
}

static void burst_once() {
  descend(10, 1);
  while (1) {
    PreemptFIFOScheduler::sleep_for_ms(1000);
  }
}

int main() {
  static Thread control_thread(1, control, 0, "control");
  static Thread producer_thread(2, producer, 1, "producer");
  static Thread consumer_thread(3, consumer, 1, "consumer");
  static Thread log_a_thread(4, logger<6, 120, 60>, 1, "log_a");
  static Thread log_b_thread(5, logger<3, 80, 90>, 1, "log_b");
  static Thread sorter_thread(6, sorter, 2, "quicksort");
  static Thread fibonacci_thread(7, fibonacci, 2, "fibonacci");
  static Thread wave_thread(8, wave<7, 80>, 2, "wave");
  static Thread danger_thread(9, wave<13, 60>, 2, "danger");
  static Thread burst_thread(10, burst_once, 2, "burst");
  static Thread monitor_thread(11, StackMonitor::routine, MAX_PRIO_LEVEL - 1, "monitor");

  PreemptFIFOScheduler::add_thread(&control_thread);
  PreemptFIFOScheduler::add_thread(&producer_thread);
  PreemptFIFOScheduler::add_thread(&consumer_thread);
  PreemptFIFOScheduler::add_thread(&log_a_thread);
  PreemptFIFOScheduler::add_thread(&log_b_thread);
  PreemptFIFOScheduler::add_thread(&sorter_thread);
  PreemptFIFOScheduler::add_thread(&fibonacci_thread);
  PreemptFIFOScheduler::add_thread(&wave_thread);
  PreemptFIFOScheduler::add_thread(&danger_thread);
  PreemptFIFOScheduler::add_thread(&burst_thread);
  PreemptFIFOScheduler::add_thread(&monitor_thread);
  PreemptFIFOScheduler::start();
  return 0;
}
