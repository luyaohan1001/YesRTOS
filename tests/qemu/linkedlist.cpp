/**
 * @file linkedlist.cpp
 * @brief linkedlist on the kernel heap: insertion order, lookup, deletion, memory returned on destruction, and nullptr
 *        (not a crash) when the heap is exhausted.
 */
#include "heap.hpp"
#include "linkedlist.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

int main() {
  itm_initialize();
  {
    linkedlist<size_t> list;
    yesrtos_test::check(list.is_empty(), "new list not empty");
    list.insert_tail(size_t(2));
    list.insert_tail(size_t(3));
    list.insert_front(size_t(1));

    size_t expected[] = {1, 2, 3};
    for (size_t i = 0; i < 3; i++) {
      list_node_t<size_t>* p_node = list[i];
      yesrtos_test::check(p_node && p_node->data == expected[i], "wrong order after insert_front/insert_tail");
    }
    yesrtos_test::check(list.lookup(size_t(2)) != nullptr, "lookup missed an element");
    list.delete_node(list.lookup(size_t(2)));
    yesrtos_test::check(list.lookup(size_t(2)) == nullptr, "deleted element still found");
    yesrtos_test::check(Heap::diagnostics().allocated > 0, "nodes not allocated on the kernel heap");
  }
  yesrtos_test::check(Heap::diagnostics().allocated == 0, "destroyed list leaked nodes");

  {
    linkedlist<size_t> list;
    uint32_t inserted = 0;
    while (list.insert_tail(size_t(inserted)) != nullptr) inserted++;
    yesrtos_test::print("nodes_until_oom=");
    yesrtos_test::print_u32(inserted);
    yesrtos_test::print("\n");
    yesrtos_test::check(inserted > 0, "no node could be allocated");
    yesrtos_test::check(list.insert_front(size_t(0)) == nullptr, "insert_front did not report out of memory");
  }
  yesrtos_test::check(Heap::diagnostics().allocated == 0, "memory not returned after out of memory");
  yesrtos_test::check(Heap::invariants_hold(), "heap invariants broken");
  yesrtos_test::pass();
}
