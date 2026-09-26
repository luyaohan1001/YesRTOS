#include "heap.hpp"

#include <cstdint>

#include "atomic_section.hpp"

// Heap region reserved by the linker script.
extern "C" {
extern uint8_t _ld_start_heap[];
extern uint8_t _ld_end_heap[];
}

using namespace YesRTOS;

/**
 * @brief The o1heap instance, created in the linker's heap region on first use. Must be called with exceptions disabled.
 * @return nullptr if the region is too small or not aligned for o1heap.
 */
O1HeapInstance* Heap::instance() {
  static O1HeapInstance* p_instance = nullptr;
  static bool initialised = false;
  if (!initialised) {
    initialised = true;
    p_instance = o1heapInit(_ld_start_heap, static_cast<size_t>(_ld_end_heap - _ld_start_heap));
  }
  return p_instance;
}

void* Heap::allocate(size_t size) {
  atomic_section a;
  O1HeapInstance* p_heap = instance();
  return p_heap ? o1heapAllocate(p_heap, size) : nullptr;
}

void Heap::free(void* p) {
  atomic_section a;
  O1HeapInstance* p_heap = instance();
  if (p_heap) {
    o1heapFree(p_heap, p);
  }
}

O1HeapDiagnostics Heap::diagnostics() {
  atomic_section a;
  O1HeapInstance* p_heap = instance();
  return p_heap ? o1heapGetDiagnostics(p_heap) : O1HeapDiagnostics{};
}

bool Heap::invariants_hold() {
  atomic_section a;
  O1HeapInstance* p_heap = instance();
  return p_heap && o1heapDoInvariantsHold(p_heap);
}
