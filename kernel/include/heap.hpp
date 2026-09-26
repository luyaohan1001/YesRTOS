/**
 * @file heap.hpp
 * @brief YesRTOS heap: the o1heap constant-time allocator (third_party/o1heap) over the linker's heap region, made
 *        thread safe.
 * @note  Every call runs inside an atomic_section. o1heap's operations take a bounded, constant time, so the section
 *        stays short and bounded. The heap is initialised on first use.
 * @note  The heap region is [_ld_start_heap, _ld_end_heap) from the linker script (_alloc_heap_size).
 */
#pragma once

#include <cstddef>

#include "o1heap.h"

namespace YesRTOS {

class Heap final {
  public:
  /**
   * @brief Allocate memory, like malloc(), in constant time.
   * @param size Number of bytes. 0 returns nullptr.
   * @return Memory aligned to O1HEAP_ALIGNMENT, or nullptr when out of memory (or the heap region is too small).
   */
  static void* allocate(size_t size);

  /**
   * @brief Release memory returned by allocate(), like free(), in constant time. nullptr is ignored.
   */
  static void free(void* p);

  /**
   * @brief Capacity, current and peak usage, out-of-memory count.
   */
  static O1HeapDiagnostics diagnostics();

  /**
   * @brief Run o1heap's consistency checks on the heap metadata.
   */
  static bool invariants_hold();

  private:
  static O1HeapInstance* instance();

  Heap() = delete;
  ~Heap() = delete;
};

}  // namespace YesRTOS
