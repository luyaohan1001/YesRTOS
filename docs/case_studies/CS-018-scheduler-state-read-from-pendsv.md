# CS-018: Scheduler state read from PendSV without masking interrupts

**Category:** Race condition  **Status:** Fixed in `edc16e5`  **Test:** none (no failing interleaving with today's handlers)

## Hazard
`schedule_next()` runs from PendSV, the lowest priority exception, so any interrupt can preempt it while it reads
`prio_bitmap`, the ready lists and the thread states. Interrupt handlers change that state: `Semaphore::release()` from
an ISR unlinks a blocked thread, appends it to a ready list and sets its `prio_bitmap` bit. Every other access to the
scheduler state was inside an `atomic_section`; this one was not. It held only because handlers merely append to ready
lists while `schedule_next()` reads list heads, an unstated ordering argument that breaks as soon as a handler removes
or moves a thread (sleep timeouts from SysTick, priority changes, suspend).

## Example
`kernel/src/preempt_fifo_scheduler.cpp` (called from PendSV, before):
```cpp
void PreemptFIFOScheduler::schedule_next() {
  uint32_t prio = count_trailing_zero<uint32_t>(prio_bitmap);
  Thread* p_next_ready = ready_list_heads[prio];    // an ISR may change the lists in between
  ...
  PreemptFIFOScheduler::p_active_thread = p_next_ready;
```

## Fix
Choose the next thread inside an `atomic_section`, from one consistent snapshot; interrupts arriving meanwhile run a
few instructions later:
```cpp
void PreemptFIFOScheduler::schedule_next() {
  atomic_section a;
  uint32_t prio = count_trailing_zero<uint32_t>(prio_bitmap);
```

## How it was found
Reviewing whether `PreemptFIFOScheduler` makes sense as a whole: listing every reader and writer of the scheduler state
showed the one access path not covered by a critical section. No test could fail on the old code, so the fix was
checked by disassembly and the existing QEMU tests.

## Design rule
Every access to state shared with interrupt handlers needs the same protection, including reads from the lowest priority
exception; do not rely on the order in which handlers happen to modify it.
