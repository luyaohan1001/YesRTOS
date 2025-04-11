#include "mempool.hpp"
#include "linkedlist.hpp"

#include <cstdint>
#include <cassert>

#include <iostream>
#include <vector>

// define heap size same to linker script (heap region capacity)
#define HEAP_SIZE 0x10000

// allow OS to allocate a region for heap testing
static size_t heap[HEAP_SIZE];

// generate heap symbol that is supposed to be defined in linker script
size_t _ld_start_heap = static_cast<size_t>(reinterpret_cast<uintptr_t>(&heap[0]));
size_t _ld_end_heap = static_cast<size_t>(reinterpret_cast<uintptr_t>(&heap[HEAP_SIZE]));

// Verify inserting front
void test_linkedlist_insert_front() {
  YesRTOS::linkedlist<size_t> list0;

  std::vector<size_t> test_vector = {1, 2, 3, 4, 5, 6, 7, 8};
  for (auto& element : test_vector) {
    list0.insert_front(element);
  }
  list0.trace_list();

  std::vector<size_t> linkedlist_content;
  list0.dump_list_to_vector(linkedlist_content);
  std::reverse(linkedlist_content.begin(), linkedlist_content.end());
  assert(test_vector == linkedlist_content);
}

// Verify inserting more than MAX_CNT_PER_SIZE_GRP for same size group
void test_linkedlist_insert_max() {
  YesRTOS::linkedlist<size_t> list0;

  // request more than max count per size group
  for (size_t i = 0; i < mempool::MAX_CNT_PER_SIZE_GRP + 100; ++i) {
    list0.insert_front(i);
  }

  std::vector<size_t> linkedlist_content;
  list0.dump_list_to_vector(linkedlist_content);

  // after inserting to the list the total number of item in the list should not exceed MAX_CNT_PER_SIZE_GRP
  assert(linkedlist_content.size() == mempool::MAX_CNT_PER_SIZE_GRP);
  list0.trace_list();
}

int main() {
  test_linkedlist_insert_front();
  test_linkedlist_insert_max();
  return 0;
}