# CS-007: Thread stacks not 8-byte aligned

**Category:** Undefined behaviour (ABI)  **Status:** Fixed in `bececfb`  **Test:** `tests/qemu/stack_alignment`

## Hazard
AAPCS requires SP to be 8-byte aligned at every public function entry. Thread stacks were plain `uint32_t` arrays
(4-byte aligned), so depending on where a `Thread` landed its routine started with `SP % 8 == 4`: two of the four demo
threads did. Doubles, `int64_t`, 8-byte aligned locals and varargs can then misbehave.

## Example
`kernel/include/thread.hpp` (before):
```cpp
uint32_t allocated_stack[STACK_ALLOCATION_SIZE];
```

## Fix
```cpp
static_assert((STACK_ALLOCATION_SIZE * sizeof(uint32_t)) % 8 == 0, "...");
alignas(8) uint32_t allocated_stack[STACK_ALLOCATION_SIZE];
```

## How it was found
Reading SP at the first instruction of each thread routine in gdb on QEMU.

## Design rule
A thread's initial stack pointer must satisfy the ABI's alignment: align the stack storage, not just its size.
