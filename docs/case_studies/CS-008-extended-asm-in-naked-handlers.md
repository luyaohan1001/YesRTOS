# CS-008: Extended asm in naked exception handlers

**Category:** Undefined behaviour (compiler dependent)  **Status:** Fixed in `c1aef96`  **Test:** none (the old code happened to work)

## Hazard
PendSV and SVC were naked functions made of several asm statements with operands. GCC only supports basic asm there:
it chooses registers for operands without knowing that `r1` holds the outgoing PSP or that `r4`-`r11` belong to a
thread, and has no frame to spill to. It worked only because the compiler picked `r3`/`r4`. `push {lr}` also left
MSP misaligned when calling C.

## Example
```cpp
__asm volatile("mrs r1, psp" ::: "r1");
__asm volatile("mov r0, %0" : : "r"(&PreemptFIFOScheduler::p_active_thread->stkptr) : "r0", "memory");
__asm volatile("str r1, [r0]" ::: "r0", "r1");   // r1 must still be PSP here
__asm volatile("push {lr}");
```

## Fix
One basic asm block per handler; the C++ work moves into `yesrtos_switch_context(sp)`, called with the stack pointer
in `r0` and `push {r3, lr}` keeping MSP 8-byte aligned. The handlers now compile identically at `-O0` and `-Os`.

## How it was found
Disassembling PendSV at `-O0` and `-Os` during a scheduler review.

## Design rule
Naked handlers contain only basic asm; call a normal C function for anything the compiler must generate.
