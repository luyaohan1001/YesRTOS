/**
 * @file stack_monitor.cpp
 * @brief Live bar chart of the thread stacks on the trace output (see stack_monitor.hpp).
 */
#include <stack_monitor.hpp>

#include <atomic_section.hpp>
#include <preempt_fifo_scheduler.hpp>

namespace YesRTOS {

namespace {

// ANSI escape sequences.
const char* const CURSOR_HOME = "\x1b[H";
const char* const CLEAR_SCREEN = "\x1b[2J";
const char* const CLEAR_TO_END_OF_LINE = "\x1b[K";
const char* const CLEAR_TO_END_OF_SCREEN = "\x1b[J";
const char* const HIDE_CURSOR = "\x1b[?25l";
const char* const RESET = "\x1b[0m";
const char* const BOLD = "\x1b[1m";
const char* const DIM = "\x1b[2m";
const char* const GREEN = "\x1b[32m";
const char* const YELLOW = "\x1b[33m";
const char* const RED = "\x1b[31m";

// Bar cells, UTF-8.
const char* const CELL_NOW = "\xe2\x96\x88";   // U+2588 full block: in use now
const char* const CELL_PEAK = "\xe2\x96\x92";  // U+2592 medium shade: used before, free now
const char* const CELL_FREE = "\xc2\xb7";      // U+00B7 middle dot: never used

struct ThreadStack {
  uint32_t id;
  const char* name;
  uint8_t priority;
  thread_state_t state;
  uint32_t used;
  uint32_t peak;
  uint32_t size;
};

void put(const char* str) {
  itm_trace(str);
}

/**
 * @brief Print `value` right aligned in `width` characters.
 */
void put_u32(uint32_t value, uint32_t width) {
  char buf[11];
  int i = sizeof(buf) - 1;
  buf[i] = '\0';
  do {
    buf[--i] = static_cast<char>('0' + value % 10);
    value /= 10;
  } while (value);
  for (uint32_t digits = sizeof(buf) - 1 - i; digits < width; digits++) put(" ");
  put(&buf[i]);
}

/**
 * @brief Print `str` left aligned in `width` characters, cut if longer.
 */
void put_padded(const char* str, uint32_t width) {
  char buf[16];
  uint32_t n = 0;
  while (n < width && n < sizeof(buf) - 1 && str[n] != '\0') {
    buf[n] = str[n];
    n++;
  }
  while (n < width && n < sizeof(buf) - 1) buf[n++] = ' ';
  buf[n] = '\0';
  put(buf);
}

const char* state_name(thread_state_t state) {
  switch (state) {
    case READY: return "READY  ";
    case RUNNING: return "RUNNING";
    case BLOCKED: return "BLOCKED";
    case SLEEP: return "SLEEP  ";
    case COMPLETE: return "DONE   ";
  }
  return "?      ";
}

/**
 * @brief Number of bar cells for `bytes` of `size`, rounded up so any use shows at least one cell.
 */
uint32_t cells(uint32_t bytes, uint32_t size) {
  return (bytes * StackMonitor::BAR_WIDTH + size - 1) / size;
}

void draw_row(const ThreadStack& t) {
  put("  ");
  if (t.name) {
    put_padded(t.name, 10);
  } else {
    // Unnamed: show the id instead, e.g. "#7".
    uint32_t width = 2;
    for (uint32_t id = t.id; id >= 10; id /= 10) width++;
    put("#");
    put_u32(t.id, 1);
    for (; width < 10; width++) put(" ");
  }
  put_u32(t.priority, 4);
  put("  ");
  put(state_name(t.state));
  put("  ");

  // Colour by high-water mark: that is what decides whether the stack is big enough.
  uint32_t percent = t.peak * 100 / t.size;
  const char* colour = percent < 50 ? GREEN : (percent < 80 ? YELLOW : RED);
  uint32_t now_cells = cells(t.used, t.size);
  uint32_t peak_cells = cells(t.peak, t.size);
  put("[");
  put(colour);
  for (uint32_t i = 0; i < StackMonitor::BAR_WIDTH; i++) {
    if (i == peak_cells) put(DIM);
    put(i < now_cells ? CELL_NOW : (i < peak_cells ? CELL_PEAK : CELL_FREE));
  }
  put(RESET);
  put("]");

  put_u32(t.used, 6);
  put_u32(t.peak, 6);
  put_u32(t.size, 6);
  put(colour);
  put_u32(percent, 5);
  put("%");
  if (t.peak >= t.size) {
    put(BOLD);
    put(" OVERFLOW");
  }
  put(RESET);
  put(CLEAR_TO_END_OF_LINE);
  put("\n");
}

}  // namespace

void StackMonitor::draw() {
  ThreadStack threads[MAX_THREADS];
  uint32_t count = 0;
  uint32_t total = 0;
  uint64_t tick;

  // Take a consistent snapshot: the registry and the saved stack pointers change on every context switch. Printing
  // is slow, so it happens afterwards with interrupts enabled.
  {
    atomic_section a;
    tick = PreemptFIFOScheduler::tick_count();
    for (Thread* p = Thread::registry_head; p; p = p->p_registry_next) {
      total++;
      if (count == MAX_THREADS) continue;
      threads[count++] = ThreadStack{p->thread_info.id, p->thread_info.name, p->thread_info.priority, p->thread_info.state,
                                     p->stack_used_bytes(), p->stack_peak_bytes(), p->stack_size_bytes()};
    }
  }

  // The registry is newest first; show the threads by id instead, idle last.
  for (uint32_t i = 1; i < count; i++) {
    ThreadStack t = threads[i];
    uint32_t j = i;
    for (; j > 0 && threads[j - 1].id > t.id; j--) threads[j] = threads[j - 1];
    threads[j] = t;
  }

  put(CURSOR_HOME);
  put(BOLD);
  put("YesRTOS stack monitor");
  put(RESET);
  put("   tick ");
  put_u32((uint32_t)tick, 1);
  put("   threads ");
  put_u32(total, 1);
  put(DIM);
  put("   (QEMU: Ctrl-A X to quit)");
  put(RESET);
  put(CLEAR_TO_END_OF_LINE);
  put("\n");
  put(CLEAR_TO_END_OF_LINE);
  put("\n");

  // Same columns as draw_row(): the legend spans the 42 characters of "[" bar "]".
  put("  THREAD    PRIO  STATE    ");
  put(CELL_NOW);
  put(" now  ");
  put(CELL_PEAK);
  put(" peak  ");
  put(CELL_FREE);
  put(" never used                  NOW  PEAK  SIZE PEAK%");
  put(CLEAR_TO_END_OF_LINE);
  put("\n");

  for (uint32_t i = 0; i < count; i++) draw_row(threads[i]);
  put(CLEAR_TO_END_OF_SCREEN);
}

void StackMonitor::routine() {
  put(HIDE_CURSOR);
  put(CLEAR_SCREEN);
  while (1) {
    draw();
    PreemptFIFOScheduler::sleep_for_ms(STACK_MONITOR_PERIOD_MS);
  }
}

}  // namespace YesRTOS
