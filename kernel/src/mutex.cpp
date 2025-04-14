#include "mutex.hpp"

namespace YesRTOS {

mutex::mutex() {
;
}

mutex::~mutex() {

}

mutex::mutex_status_t mutex::try_lock() {
  return OP_SUCCESS;
}


mutex::mutex_status_t mutex::try_unlock() {
  return OP_SUCCESS;
}

}