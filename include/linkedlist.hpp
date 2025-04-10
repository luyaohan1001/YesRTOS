#include <cassert>
#include <iostream>
#include <utility>  // for std::move
#include <vector>

#include "mempool.hpp"

namespace YesRTOS {

template <typename T>
struct list_node {
  T data;
  struct list_node* next;

  list_node(T init_val) : data(init_val), next(nullptr) {
  }
  list_node(T init_val, struct list_node* next) : data(init_val), next(next) {
  }
};
template <typename T>
using list_node_t = struct list_node<T>;

template <typename T>
class linkedlist final {
  public:
  linkedlist() : head(nullptr){};
  ~linkedlist();

  list_node_t<T>* head;
  list_node_t<T>* insert_front(T& data);
  list_node_t<T>* insert_front(T&& data);

  void delete_node(list_node_t<T>* p_del_target);
  bool is_empty() const;
  list_node_t<T>* lookup(T&& data) const;
  list_node_t<T>* lookup(list_node_t<T>* p_target_node) const;
  list_node<T>* get_next_node_circular(list_node_t<T>* p_curr_node) const;
  void dump_list_to_vector(std::vector<T>& v);
  void trace_list() const;

  // overload [] operator to allow array-link access syntax for linkedlist.
  list_node_t<T>* operator[](size_t& index) const;
};

}  // namespace YesRTOS