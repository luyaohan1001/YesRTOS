# CS-012: add_thread() without validation or locking

**Category:** Data integrity  **Status:** Fixed in `2f7410e`  **Test:** `tests/qemu/add_thread`

## Hazard
`add_thread()` indexed `ready_list_heads[priority]` with an unchecked `uint8_t`, dereferenced null threads, and linked
threads into ready lists with exceptions enabled, racing PendSV after `start()`. `1 << bitpos` overflowed a signed int
at bit 31.

## Example
```cpp
uint8_t prio_level = p_new->thread_info.priority;
Thread** pp_head = &ready_list_heads[prio_level];   // prio_level up to 255
```

## Fix
Return `false` for null threads and priorities `>= MAX_PRIO_LEVEL`; link inside an `atomic_section`; shift `T(1)`.

## How it was found
Scheduler review.

## Design rule
Validate every index that comes from the API before using it, and treat public kernel calls as callable at any time.
