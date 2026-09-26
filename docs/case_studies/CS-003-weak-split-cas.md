# CS-003: Compare-and-swap split across functions

**Category:** Atomicity  **Status:** Fixed in `c1c34b6`  **Test:** `tests/qemu/cas_spinlock` (does not reproduce the old failure on QEMU)

## Hazard
LDREX and STREX lived in two C functions, so compiler generated memory accesses (stack spills, returns) could land
between them. A lost reservation (any exception between LDREX and STREX) made the CAS return false although the value
never changed, which callers read as "value differs". The implementation also took `bool*` while the header declared
`volatile uint32_t*`.

## Example
`kernel/arch/armv7m/atomic.cpp` (before):
```cpp
bool atomic_compare_and_swap(uint32_t *p_mem, uint32_t old_val, uint32_t new_val) {
  uint32_t load_val = exclusive_load(p_mem);     // LDREX in another function
  if (old_val != load_val) return false;          // exclusive state left open
  return exclusive_store(p_mem, new_val);         // STREX may fail spuriously
}
```

## Fix
One asm block: LDREX, compare, STREX, retry on a lost reservation, CLREX on mismatch, DMB on success. `false` then
always means the value differed.

## How it was found
Code review. Neither version failed a stress test on QEMU, whose exclusive monitor is more permissive than hardware.

## Design rule
Keep an LDREX/STREX pair in a single asm block, and retry lost reservations so a failed CAS always means a different value.
