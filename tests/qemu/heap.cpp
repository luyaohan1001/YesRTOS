/**
 * @file heap.cpp
 * @brief YesRTOS::Heap: alignment, data integrity, out-of-memory handling and full release on one thread, then
 *        concurrent allocate/free from four preempted threads.
 */
#include "atomic_section.hpp"
#include "heap.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static const uint32_t THREADS = 4;
static const uint32_t ROUNDS = 3000;
static volatile uint32_t finished = 0;

static void fill(void* p, size_t size, uint8_t value) {
  for (size_t i = 0; i < size; i++) static_cast<volatile uint8_t*>(p)[i] = value;
}

static bool holds(const void* p, size_t size, uint8_t value) {
  for (size_t i = 0; i < size; i++) {
    if (static_cast<const volatile uint8_t*>(p)[i] != value) return false;
  }
  return true;
}

static void single_thread_checks() {
  yesrtos_test::check(Heap::allocate(0) == nullptr, "allocate(0) returned memory");

  static const size_t sizes[] = {1, 7, 8, 24, 100, 512, 1000};
  void* blocks[sizeof(sizes) / sizeof(sizes[0])];
  for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
    blocks[i] = Heap::allocate(sizes[i]);
    yesrtos_test::check(blocks[i] != nullptr, "small allocation failed");
    yesrtos_test::check(reinterpret_cast<uintptr_t>(blocks[i]) % 8 == 0, "allocation not 8-byte aligned");
    fill(blocks[i], sizes[i], static_cast<uint8_t>(0xA0 + i));
  }
  for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
    yesrtos_test::check(holds(blocks[i], sizes[i], static_cast<uint8_t>(0xA0 + i)), "allocations overlap");
    Heap::free(blocks[i]);
  }

  // Exhaust the heap with 256-byte blocks, then give everything back.
  void* all[64];
  uint32_t count = 0;
  while (count < 64 && (all[count] = Heap::allocate(256)) != nullptr) count++;
  yesrtos_test::check(count > 0 && count < 64, "heap is not bounded by the linker region");
  yesrtos_test::check(Heap::diagnostics().oom_count > 0, "out-of-memory not counted");
  for (uint32_t i = 0; i < count; i++) Heap::free(all[i]);
  Heap::free(nullptr);

  O1HeapDiagnostics d = Heap::diagnostics();
  yesrtos_test::print("capacity=");
  yesrtos_test::print_u32(d.capacity);
  yesrtos_test::print(" blocks_of_256=");
  yesrtos_test::print_u32(count);
  yesrtos_test::print("\n");
  yesrtos_test::check(d.allocated == 0, "memory still allocated after freeing everything");
  yesrtos_test::check(Heap::invariants_hold(), "heap invariants broken");
}

static void worker() {
  uint8_t tag = static_cast<uint8_t>(PreemptFIFOScheduler::p_active_thread->thread_info.id + 1);
  uint32_t seed = tag * 2654435761u;
  for (uint32_t round = 0; round < ROUNDS; round++) {
    seed = seed * 1664525u + 1013904223u;
    size_t size = 8 + (seed >> 24) % 200;
    void* p = Heap::allocate(size);
    if (!p) continue;  // the other threads may hold the rest of the heap
    fill(p, size, tag);
    PreemptFIFOScheduler::yield();  // let the others allocate and free around this block
    yesrtos_test::check(holds(p, size, tag), "block modified by another thread");
    Heap::free(p);
  }

  atomic_section a;
  if (++finished == THREADS) {
    yesrtos_test::check(Heap::diagnostics().allocated == 0, "memory leaked under concurrency");
    yesrtos_test::check(Heap::invariants_hold(), "heap invariants broken under concurrency");
    yesrtos_test::pass();
  }
}

int main() {
  itm_initialize();
  single_thread_checks();

  static Thread t0(0, worker), t1(1, worker), t2(2, worker), t3(3, worker);
  PreemptFIFOScheduler::add_thread(&t0);
  PreemptFIFOScheduler::add_thread(&t1);
  PreemptFIFOScheduler::add_thread(&t2);
  PreemptFIFOScheduler::add_thread(&t3);
  PreemptFIFOScheduler::start();
  return 0;
}
