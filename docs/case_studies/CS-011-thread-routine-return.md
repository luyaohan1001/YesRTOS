# CS-011: Thread routine returning into 0xDEADBEEF

**Category:** Undefined behaviour  **Status:** Fixed in `34446ac`  **Test:** `tests/qemu/thread_exit`

## Hazard
The initial frame set LR to `0xDEADBEEF`, so a routine that returned branched there and HardFaulted. Thread states
were unreliable too: nothing set RUNNING.

## Example
`kernel/arch/armv7m/timeslice.cpp` (before):
```cpp
(**pp_stk) = 0xDEADBEEF;   // LR of the initial frame
```

## Fix
The initial LR is `yesrtos_thread_exit()`, which moves the thread to a completed list, marks it COMPLETE and switches
away for good; `schedule_next()` keeps RUNNING/READY current.

## How it was found
Scheduler review.

## Design rule
Every thread needs a defined way to end: point its initial return address at an exit routine.
