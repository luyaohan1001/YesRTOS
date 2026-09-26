# CS-002: Scheduling through a blocked thread's list links

**Category:** Data integrity  **Status:** Fixed in `1970288`  **Test:** `tests/qemu/thread_exit`, `tests/qemu/mutex_exclusion`

## Hazard
When the running thread blocked, `block_running_thread()` moved it into a blocked list, but `schedule_next()` still
rotated from it: its `p_next` now pointed into the blocked list, so another blocked thread could be scheduled.
`unblock_one_thread()` also used the *running* thread's priority and set the *running* thread READY instead of the
woken one.

## Example
`kernel/src/preempt_fifo_scheduler.cpp` (before):
```cpp
if (prio == p_active_thread->thread_info.priority) {
  p_next_ready = get_next_thread_circular(p_active_thread, ready_list_heads[prio]);

Thread *p_thread = PreemptFIFOScheduler::p_active_thread;   // in unblock_one_thread()
move_node(pp_blocked_list_head, &ready_list_heads[p_thread->thread_info.priority], *pp_blocked_list_head);
p_thread->thread_info.state = READY;
```

## Fix
Only rotate from the active thread while it is still in a ready list (RUNNING or READY; `34446ac` added COMPLETE to
the excluded states). Wake the tail of the blocked list with that thread's own priority and state.

## How it was found
Code review while fixing CS-001.

## Design rule
A node moved to another list must not be traversed through its old links; act on the node you moved, not on the current thread.
