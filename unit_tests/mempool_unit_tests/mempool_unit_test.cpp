#include "mempool.hpp"
#include <cstdint>
#include <cassert>
#include <iostream>
#include <string>

// define heap size same to linker script (heap region capacity)
#define HEAP_SIZE 0x10000

// allow OS to allocate a region for heap testing
static size_t heap[HEAP_SIZE];

// generate heap symbol that is supposed to be defined in linker script
size_t _ld_start_heap = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&heap[0]));
size_t _ld_end_heap = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&heap[HEAP_SIZE]));

// Testing allocation size < 32
void test_alloc_size_lt_32() {
  auto test_req_size = 12;
  mempool::alloc_t alloc_res = {mempool::ALLOC_FAIL, nullptr};
  mempool::init();

  for (auto loop = 0; loop < mempool::MAX_CNT_PER_SIZE_GRP; ++loop) {
    alloc_res = mempool::malloc(test_req_size);
    mempool::size_group_t sz_grp = mempool::match_size_grp(test_req_size);
    assert(alloc_res.status == mempool::ALLOC_SUCCESS);
  }

  // Expect allocation failure after reaching MAX amount it can allocate to the size group (size_group_t).
  alloc_res = mempool::malloc(test_req_size);
  assert(alloc_res.status == mempool::ALLOC_FAIL);
}

// Testing allocation + free in size < 32
void test_alloc_free_size_lt_32() {
  auto test_req_size = 12;
  mempool::alloc_t alloc_res = {mempool::ALLOC_FAIL, nullptr};
  mempool::init();

  for (auto loop = 0; loop < mempool::MAX_CNT_PER_SIZE_GRP; ++loop) {
    alloc_res = mempool::malloc(test_req_size);
    mempool::size_group_t sz_grp = mempool::match_size_grp(test_req_size);
    assert(alloc_res.status == mempool::ALLOC_SUCCESS);
    mempool::free(alloc_res.addr);
    assert(mempool::get_alloc_bitmap(sz_grp) == 0);
  }
}

// Testing allocation invalid size
void test_alloc_size_0xFF() {
  mempool::init();

  auto alloc_res = mempool::malloc(0xFF);
  assert(alloc_res.status == mempool::ALLOC_FAIL);
}


// Testing allocation size == 128
void test_alloc_size_128() {
  mempool::init();

  auto alloc_res = mempool::malloc(128);
  mempool::size_group_t sz_grp = mempool::match_size_grp(128);
  assert(alloc_res.status == mempool::ALLOC_SUCCESS);

  assert(mempool::get_alloc_bitmap(mempool::SIZE_GRP_128_BYTE) == 1);
}

int main(int argc, char** argv) {
  std::string in_str(argv[0]);
  if (in_str == std::string("alloc size < 32")) {
    test_alloc_size_lt_32();
  } else if (in_str == std::string("alloc and free size < 32")) {
    test_alloc_free_size_lt_32();
  } else if (in_str == std::string("alloc size 128")) {
    test_alloc_size_128();
  } else if (in_str == std::string("invalid size")) {
    test_alloc_size_0xFF();
  }
  return 0;
}