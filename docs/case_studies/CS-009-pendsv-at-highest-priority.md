# CS-009: PendSV and SysTick at the highest priority

**Category:** Interrupt priority  **Status:** Fixed in `1099c2a`  **Test:** `tests/qemu/exception_priority`

## Hazard
Nothing set the system handler priorities, so PendSV and SysTick kept reset priority 0, the highest configurable one.
A pending context switch could preempt any interrupt handler and swap thread stacks underneath it.

## Example
After `start()` a thread read `PendSV=0 SysTick=0` from SHPR3 (`0xE000ED20`).

## Fix
```cpp
SHPR3 |= (0xFFUL << 16UL) | (0xFFUL << 24UL);   // lowest priority, whatever bits are implemented
```

## How it was found
Scheduler review; confirmed by reading SHPR3 on QEMU.

## Design rule
The context switch exception runs at the lowest priority, after every interrupt handler has finished.
