#include "mempool.hpp"
#include "linkedlist.hpp"
#include <cstdint>
#include <cassert>
#include <iostream>

// define heap size same to linker script (heap region capacity)
#define HEAP_SIZE 0x10000

// allow OS to allocate a region for heap testing
static size_t heap[HEAP_SIZE];

// generate heap symbol that is supposed to be defined in linker script
size_t _ld_start_heap = static_cast<size_t>(reinterpret_cast<uintptr_t>(&heap[0]));
size_t _ld_end_heap = static_cast<size_t>(reinterpret_cast<uintptr_t>(&heap[HEAP_SIZE]));

int main() {
  mempool::init();
  linkedlist<size_t> list0;
  list0.insert_front(10);
  list0.insert_front(133);
  list0.insert_front(1);
  list0.insert_front(13);
  list0.insert_front(2);
  list0.trace_list();

  list_node_t<size_t>* node = list0.lookup(2);
  list0.delete_node(node);
  // list0.insert_front(50);
  list0.trace_list();

  return 0;
}