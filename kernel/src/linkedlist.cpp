#include "linkedlist.hpp"

#include <new>  // placement new

#include "thread.hpp"

namespace YesRTOS {

/**
 * @brief Allocate a node on the kernel heap and construct it in place.
 * @return nullptr when the heap is exhausted.
 */
template <typename T>
static list_node_t<T>* new_node(T&& data, list_node_t<T>* next) {
  void* p_mem = Heap::allocate(sizeof(list_node_t<T>));
  if (!p_mem) {
    return nullptr;
  }
  return new (p_mem) list_node_t<T>(std::move(data), next);
}

/**
 * @brief Destroy a node created by new_node() and return its memory to the heap.
 */
template <typename T>
static void delete_list_node(list_node_t<T>* p_node) {
  p_node->~list_node_t<T>();
  Heap::free(p_node);
}

template <typename T>
linkedlist<T>::linkedlist() : head(nullptr) {
}

template <typename T>
linkedlist<T>::~linkedlist() {
  list_node_t<T>* p_node_itr = this->head;
  // single node list
  if (this->is_empty()) return;

  while (p_node_itr) {
    list_node_t<T>* p_next_node = p_node_itr->next;
    delete_list_node(p_node_itr);
    p_node_itr = p_next_node;
  }
  this->head = nullptr;
}

template <typename T>
bool linkedlist<T>::is_empty() const {
  return this->head == nullptr;
}

template <typename T>
list_node_t<T>* linkedlist<T>::insert_front(T& data) {
  return insert_front(std::move(data));  // moves lvalue -> rvalue
}

template <typename T>
list_node_t<T>* linkedlist<T>::insert_front(T&& data) {
  list_node_t<T>* p_new_node = new_node(std::move(data), this->head);
  if (!p_new_node) {
    return nullptr;
  }
  this->head = p_new_node;
  return p_new_node;
}

template<typename T>
list_node_t<T>* linkedlist<T>::insert_tail(T& data) {
  return insert_tail(std::move(data));
}

template<typename T>
list_node_t<T>* linkedlist<T>::insert_tail(T&& data) {
  list_node_t<T>* p_new_node = new_node(std::move(data), static_cast<list_node_t<T>*>(nullptr));
  if (!p_new_node) {
    return nullptr;
  }

  if (head) {
    list_node_t<T>* p_node_itr = head;
    while (p_node_itr->next) p_node_itr = p_node_itr->next;
    p_node_itr->next = p_new_node;
  } else {
    head = p_new_node;
  }
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
      delete_list_node(p_node_itr);
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
list_node<T>* linkedlist<T>::get_next_node_circular(list_node_t<T>* p_curr_node) const {
  if (!p_curr_node)
    return nullptr;
  else if (!p_curr_node->next) {
    return this->head;
  } else {
    return p_curr_node->next;
  }
}

template <typename T>
list_node_t<T>* linkedlist<T>::operator[](size_t& index) const {
  list_node_t<T>* p_node_itr = this->head;
  size_t cnt = 0;
  while (p_node_itr) {
    if (cnt++ == index) {
      return p_node_itr;
    }
    p_node_itr = p_node_itr->next;
  }
  return nullptr;
}

#if defined(HOST_PLATFORM)
template <typename T>
void linkedlist<T>::dump_list_to_vector(std::vector<T>& v) {
  list_node_t<T>* p_node_itr = this->head;
  while (p_node_itr) {
    v.push_back(p_node_itr->data);
    p_node_itr = p_node_itr->next;
  }
}
#endif

#if defined(HOST_PLATFORM)
template <typename T>
void linkedlist<T>::trace_list() const {
  list_node_t<T>* p_node_itr = this->head;
  while (p_node_itr) {
    std::cout << p_node_itr->data << " -> ";
    p_node_itr = p_node_itr->next;
  }
  std::cout << "[NULL]" << std::endl;
}
#endif

// explicit instantiation of template (does not allocate memory)
template class linkedlist<size_t>;
template struct list_node<size_t>;

template class linkedlist<YesRTOS::Thread>;
template struct list_node<YesRTOS::Thread>;

}  // namespace YesRTOS