# CS-010: No thread ready: count_trailing_zero(0)

**Category:** Undefined behaviour  **Status:** Fixed in `1ff6d5b`  **Test:** `tests/qemu/idle_when_all_blocked`

## Hazard
With every thread blocked, `prio_bitmap == 0` and the scheduler evaluated `__builtin_ctz(0)` (undefined; 32 on
Cortex-M), then read `ready_list_heads[32]` far past the array.

## Example
```cpp
uint32_t prio = count_trailing_zero<uint32_t>(prio_bitmap);   // prio_bitmap == 0
p_next_ready = ready_list_heads[prio];
```

## Fix
A kernel-owned idle thread at a reserved lowest priority is always ready; it calls a weak `yesrtos_idle_hook()` and
sleeps with `wfi`.

## How it was found
Scheduler review. The test deadlocks two threads on two mutexes; its first version was itself flaky under the 10 kHz tick and was fixed in `5cc6cb8`.

## Design rule
The ready set must never be empty: give the scheduler an idle thread.
