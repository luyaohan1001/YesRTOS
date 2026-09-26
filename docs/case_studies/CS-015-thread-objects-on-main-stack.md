# CS-015: Thread objects on the main stack, no stack reserve

**Category:** Stack overflow  **Status:** **Open**  **Test:** none

## Hazard
The demo creates its `Thread` objects (each with a 1600 B stack) as locals of `main()`, so ~6.5 KB of thread stacks
live on MSP, the stack also used by every exception handler. The linker scripts reserve `_alloc_stack_size = 0x0`,
so nothing checks at link time that MSP has room, and nothing detects an overflow at run time.

## Example
`app/multi_thread/main.cpp`:
```cpp
int main() {
  Thread thread0(0, thread0_routine, 0);   // 1628 B on MSP
```

## Fix
Not fixed yet. Give threads static storage, reserve an explicit MSP size in the linker scripts, and consider
stack painting plus an MPU guard region for overflow detection.

## How it was found
`yesrtos stacks` (tools/gdb/stack-usage.py) reported `MSP 6584 B below _estack`.

## Design rule
Size and reserve every stack explicitly, including the exception stack; do not put thread stacks on it.
