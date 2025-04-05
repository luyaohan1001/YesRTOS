#include "linkedlist.hpp"

template <typename T>
linkedlist<T>::~linkedlist() {
  list_node_t<T>* p_node_itr = this->head;
  // single node list
  if (this->is_empty()) return;

  while (p_node_itr) {
    list_node_t<T>* p_next_node = p_node_itr->next;
    mempool::free(reinterpret_cast<size_t*>(p_node_itr));
    p_node_itr = p_next_node;
  }
  this->head = nullptr;
}

template <typename T>
bool linkedlist<T>::is_empty() const {
  return this->head == nullptr;
}

template <typename T>
list_node_t<T>* linkedlist<T>::insert_front(T&& data) {
  mempool::alloc_t alloc_res = mempool::malloc(sizeof(list_node_t<T>));
  if (alloc_res.status == mempool::ALLOC_FAIL) {
    return nullptr;
  }
  list_node_t<T>* p_new_node = reinterpret_cast<list_node_t<T>*>(alloc_res.addr);
  p_new_node->data = data;
  p_new_node->next = head;
  this->head = p_new_node;
  return p_new_node;
}

template <typename T>
void linkedlist<T>::delete_node(list_node_t<T>* p_del_target) {
  if (this->is_empty()) return;
  list_node_t<T>* p_node_itr = this->head;
  list_node_t<T>* p_prev_node = nullptr;
  while (p_node_itr) {
    if (p_node_itr == p_del_target) {
      if (p_prev_node)
        p_prev_node->next = p_node_itr->next;
      else
        this->head = p_node_itr->next;
      mempool::free(reinterpret_cast<size_t*>(p_node_itr));
      break;
    }
    p_prev_node = p_node_itr;
    p_node_itr = p_node_itr->next;
  }
}

template <typename T>
list_node_t<T>* linkedlist<T>::lookup(T&& data) const {
  list_node_t<T>* p_node_itr = this->head;
  while (p_node_itr) {
    if (p_node_itr->data == data) {
      return p_node_itr;
    }
    p_node_itr = p_node_itr->next;
  }
  return nullptr;
}

template <typename T>
list_node_t<T>* linkedlist<T>::lookup(list_node_t<T>* p_target_node) const {
  list_node_t<T>* p_node_itr = this->head;
  while (p_node_itr) {
    if (p_node_itr->data == p_target_node->data) {
      return p_node_itr;
    }
    p_node_itr = p_node_itr->next;
  }
  return nullptr;
}

template <typename T>
list_node_t<T>* linkedlist<T>::operator[](T& index) const {
  list_node_t<T>* p_node_itr = this->head;
  T cnt = 0;
  while (p_node_itr) {
    if (cnt++ == index) {
      return p_node_itr;
    }
    p_node_itr = p_node_itr->next;
  }
  return nullptr;
}

template <typename T>
void linkedlist<T>::trace_list() const {
  list_node_t<T>* p_node_itr = this->head;
  while (p_node_itr) {
    std::cout << p_node_itr->data << " -> ";
    p_node_itr = p_node_itr->next;
  }
  std::cout << "[NULL]" << std::endl;
}

// explicit instantiation
template class linkedlist<size_t>;
template struct list_node<size_t>;