#include <cstddef> // size_t
#include "baremetal_api.h"

namespace YesRTOS {

class mutex {
  public:
    typedef enum {
      OP_SUCCESS = 0,
      OP_FAIL = 1
    } op_res_t;

    mutex();
    ~mutex();
    op_res_t try_lock();
    op_res_t try_unlock();

  public:
    constexpr static size_t UNLOCKED = 0;
    constexpr static size_t LOCKED = 0;

  private:
    bool atomic_val;
};

}
