# CS-006: Spinlock across priorities on a single core

**Category:** Deadlock  **Status:** Documented  **Test:** none

## Hazard
On one core a spinning thread only wastes its timeslice until it is preempted. If a higher priority thread spins on a
lock held by a lower priority thread, the holder never runs again and the system deadlocks.

## Example
```cpp
while (!atomic_compare_and_swap(&this->locked, 0, 1)) {
}   // a higher priority spinner never lets the lower priority owner run
```

## Fix
No code fix: `spinlock` is documented as equal-priority only. Threads use `Mutex` (blocking); the kernel itself uses
`atomic_section`.

## How it was found
Design discussion while rebuilding the spinlock on the kernel CAS (`c1c34b6`).

## Design rule
On a single core, block instead of spinning; spinlocks belong to multi-core code.
