# CS-013: Linked list nodes used without construction

**Category:** Undefined behaviour  **Status:** Fixed in `e5b77ee`  **Test:** `tests/qemu/linkedlist`

## Hazard
`linkedlist` assigned into raw pool memory without constructing the node (undefined for types such as `Thread`),
freed nodes without destroying them, and on allocation failure hit `assert(0)` and then dereferenced null.

## Example
```cpp
list_node_t<T>* p_new_node = reinterpret_cast<list_node_t<T>*>(alloc_res.addr);
p_new_node->data = data;
```

## Fix
Allocate from `YesRTOS::Heap`, construct with placement new, destroy before freeing, return `nullptr` on exhaustion.

## How it was found
Porting the list from mempool to the o1heap based heap.

## Design rule
Memory from an allocator is not an object until it is constructed; an allocation failure is a return value, not an assert.
