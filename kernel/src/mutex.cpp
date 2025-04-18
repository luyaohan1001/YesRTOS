#include "mutex.hpp"

namespace YesRTOS {

mutex::mutex() {
  this->atomic_val = false;
}

mutex::~mutex() {
}


mutex::op_res_t mutex::try_lock() {
  size_t ret = atomic_compare_and_swap(&this->atomic_val, mutex::UNLOCKED, mutex::LOCKED);
  return static_cast<mutex::op_res_t>(ret);
}

mutex::op_res_t mutex::try_unlock() {
  size_t ret = atomic_compare_and_swap(&this->atomic_val, mutex::LOCKED, mutex::UNLOCKED);
  return static_cast<mutex::op_res_t>(ret);
}

}