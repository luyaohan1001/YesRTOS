# CS-005: Disabling interrupts is not a compiler barrier

**Category:** Memory ordering  **Status:** Fixed in `334c620`  **Test:** none

## Hazard
`cpsid i` / `cpsie i` had no `"memory"` clobber, so the compiler could move loads and stores of shared data across
them. It only held because the functions were compiled in another translation unit; inlining or LTO would break
every critical section.

## Example
```cpp
__asm volatile("cpsid i");
```

## Fix
```cpp
__asm volatile("cpsid i" ::: "memory");
__asm volatile("cpsie i" ::: "memory");
```

## How it was found
Code review.

## Design rule
Every instruction that starts or ends a critical section must also be a compiler barrier.
