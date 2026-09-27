# CS-016: Context switch requested before the first thread runs

**Category:** Race condition  **Status:** Fixed in `487f805`  **Test:** `tests/qemu/startup_pending_switch`

## Hazard
`start()` enabled SysTick and only then executed `svc 0` to run the first thread. A tick in between pended PendSV,
which then ran from `main()`: PSP did not point at any thread stack yet, so the handler stored `r4`-`r11` below address
0 and HardFaulted. The window is a few instructions wide; it was hit only with a fast tick on a loaded host
(`ctest -j8`), as sporadic test timeouts.

## Example
`kernel/src/preempt_fifo_scheduler.cpp`, `start()` (before):
```cpp
kernel_exception_priority_init();
systick_clk_init();     // SysTick may fire from here on ...
start_first_task();     // ... before `svc 0` sets up PSP
```

## Fix
PendSV returns immediately until the SVC handler has switched to the first thread (`yesrtos_first_context()` sets
the flag). This covers any switch requested before `start()`, not only the tick:
```asm
ldr    r0, =yesrtos_scheduler_started
ldr    r0, [r0]
cbz    r0, 1f          // scheduler not started: nothing to switch from
...
1:
bx     lr
```

## How it was found
Tests timing out only under `ctest -j8`; attaching gdb to a hung QEMU instance showed PendSV called from
`start_first_task()`. The test makes the window deterministic by requesting a switch from `main()` before `start()`.

## Design rule
Until the first thread runs there is no context to switch from: the context switch handler must be a no-op before start.
