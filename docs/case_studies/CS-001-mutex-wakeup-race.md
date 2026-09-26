# CS-001: Mutex wake-up without ownership

**Category:** Race condition  **Status:** Fixed in `c4067fe`  **Test:** `tests/qemu/mutex_exclusion`

## Hazard
Taking the mutex was a test of `locked` followed by a separate store, with exceptions enabled. A thread preempted
between the two lets another thread take the mutex too, and both run inside the critical section. Woken threads
spun on `locked` instead of being handed the mutex, and the scheduler lists were modified with exceptions enabled.
An uncommitted rewrite made the window wide: its woken thread entered without setting `locked` at all, producing
54-68 interleaved trace lines per QEMU run.

## Example
`kernel/src/mutex.cpp` (before):
```cpp
if (locked == true) {
  PreemptFIFOScheduler::block_running_thread(&this->p_blocked_list);
  request_context_switch();
}
while(locked == true);   // woken threads spin here
locked = true;           // test above and store here are not atomic
```

## Fix
Never let a woken thread race for the lock. `unlock()` hands ownership straight to the longest waiter and keeps the
mutex locked, so nobody can take it in between; list changes happen inside an `atomic_section`:
```cpp
if (this->p_blocked_list) {
  this->owner = PreemptFIFOScheduler::unblock_one_thread(&this->p_blocked_list);
  request_context_switch();
}
```
(Later `2c38498` added a CAS fast path for the uncontended case.)

## How it was found
Interleaved lines such as `thread thread 1` in the QEMU demo output. The first regression test did not catch it:
threads mostly took turns and blocked only ~15 times per run. The test now yields inside the critical section to force
contention on every iteration, and fails immediately on the broken version.

## Design rule
Hand a released lock directly to a waiter; test lock implementations by forcing contention, not by hoping for it.
