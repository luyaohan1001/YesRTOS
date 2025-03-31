#include "mempool.hpp"
#include <cstdint>
#include <cassert>
#include <iostream>

// define heap size same to linker script (heap region capacity)
#define HEAP_SIZE 0x10000

// allow OS to allocate a region for heap testing
static uint32_t heap[HEAP_SIZE];

// generate heap symbol that is supposed to be defined in linker script
uint32_t _ld_start_heap = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&heap[0]));
uint32_t _ld_end_heap = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&heap[HEAP_SIZE]));

void test_malloc_alloc_size_variation() {
  uint32_t test_req_size = 12;
  mempool::init();

  for (uint32_t sz_sweep_var = 0; sz_sweep_var < 10; ++sz_sweep_var) {
    mempool::alloc_t alloc_res;
    alloc_res = mempool::malloc(sz_sweep_var);
    mempool::size_group_t sz_grp = mempool::match_size_grp(test_req_size);
    assert(alloc_res.status == mempool::ALLOC_SUCCESS);
    mempool::free(alloc_res.addr);
  }
}

int main() {
  test_malloc_alloc_size_variation();
  return 0;
}