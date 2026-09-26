# CS-014: FPU context not saved on context switch

**Category:** Data integrity  **Status:** **Open**  **Test:** none

## Hazard
Both targets are built with a hardware FPU (`-mfloat-abi=hard`). PendSV saves only `r4`-`r11`: when threads use
floating point, `s16`-`s31` of one thread leak into another, and the extended exception frame (EXC_RETURN bit 4 = 0)
is not handled per thread. The FPU is also never enabled (no CPACR write; the startup `SystemInit` call is commented
out), so the first floating point instruction raises a UsageFault.

## Example
`kernel/arch/armv7m/timeslice.cpp`:
```asm
stmdb  r0!, {r4-r11}   // s16-s31 are not saved
```

## Fix
Not fixed yet. Enable the FPU in CPACR at startup, save `s16`-`s31` when EXC_RETURN bit 4 is 0, and keep each thread's
EXC_RETURN in its saved context; size thread stacks for the extended frame.

## How it was found
Comparing Cortex-M3 and Cortex-M7 from an RTOS point of view.

## Design rule
If threads may use the FPU, the FPU registers are part of the thread context.
