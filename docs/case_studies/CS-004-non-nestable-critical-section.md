# CS-004: Critical section re-enables interrupts when nested

**Category:** Race condition  **Status:** Fixed in `a57be71`  **Test:** `tests/qemu/critical_section_nesting`

## Hazard
`atomic_section` re-enabled exceptions unconditionally on exit. An inner section (e.g. `Mutex::unlock()` called
inside a caller's section) turned exceptions back on and the rest of the outer section ran unprotected.

## Example
```cpp
atomic_section::atomic_section()  { disable_exception(); }
atomic_section::~atomic_section() { enable_exception(); }   // PRIMASK -> 0 even when nested
```

## Fix
Save PRIMASK on entry and restore it on exit:
```cpp
atomic_section::atomic_section()  { this->saved_primask = save_and_disable_exception(); }
atomic_section::~atomic_section() { restore_exception(this->saved_primask); }
```
Consequence: blocking calls must not be made inside a critical section; `Mutex::lock()` now refuses to block there.

## How it was found
Review of how the mutex uses critical sections.

## Design rule
Restore the saved interrupt state instead of forcing it on; never block inside a critical section.
