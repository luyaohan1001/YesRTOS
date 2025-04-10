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
    assert(get_alloc_bitmap(sz_grp) == 0);
  }
}

// Testing allocation invalid size
void test_alloc_invalid_size() {
  mempool::init();

  auto alloc_res = mempool::malloc(-1);
  assert(alloc_res.status == mempool::ALLOC_FAIL);

  alloc_res = mempool::malloc(9999999999999);
  assert(alloc_res.status == mempool::ALLOC_FAIL);
}


// Testing allocation size == 128
void test_alloc_size_128() {
  mempool::init();

  auto alloc_res = mempool::malloc(128);
  mempool::size_group_t sz_grp = mempool::match_size_grp(128);
  assert(alloc_res.status == mempool::ALLOC_SUCCESS);

  assert(get_alloc_bitmap(mempool::SIZE_GRP_128_BYTE) == 1);
}

// Testing allocating more than max counter per size group
void test_alloc_more_than_max_cnt() {
  mempool::init();
  auto test_req_size = 0x30;

  mempool::alloc_t alloc_res = {mempool::ALLOC_FAIL, nullptr};
  for (auto loop = 0; loop < mempool::MAX_CNT_PER_SIZE_GRP; ++loop) {
    alloc_res = mempool::malloc(test_req_size);
    assert(alloc_res.status == mempool::ALLOC_SUCCESS);
  }

  alloc_res = mempool::malloc(test_req_size);
  assert(alloc_res.status == mempool::ALLOC_FAIL);
}

void test_init_address() {
  mempool::init();

  size_t* p_alloc_bitmap = get_size_grp_start_addr();
  size_t alloc_size_groups = get_alloc_size_groups();
  for (int i = 0; i < alloc_size_groups; ++i) {
    assert(p_alloc_bitmap[i] != 0); // check address initialization
    assert(p_alloc_bitmap[i] >= _ld_start_heap); // check address validity
    assert(p_alloc_bitmap[i] < _ld_end_heap); // check address validity
  }
}

int main(int argc, char** argv) {
  if (!argv[1]) {
    std::cerr << "Usage: Run 'ctest' OR './mempool_unit_test <TEST_ID>'" << std::endl;
    exit(1);
  }

  std::string in_str(argv[1]);
  auto unit_test_id = std::stoi(in_str);
  switch (unit_test_id) {
    case 0:
      test_alloc_size_lt_32();
      break;
    case 1:
      test_alloc_free_size_lt_32();
      break;
    case 2:
      test_alloc_size_128();
      break;
    case 3:
      test_alloc_invalid_size();
      break;
    case 4:
      test_alloc_more_than_max_cnt();
      break;
    case 5:
      test_init_address();
      break;
    default: break;
  }
  return 0;
}