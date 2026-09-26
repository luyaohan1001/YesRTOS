#include <cstddef>  // size_t
#include <cstdint>
#include <atomic>

#if defined(ARMV7M)
#include "baremetal_api.h"
#endif

namespace YesRTOS {

class spinlock {
  public:

  spinlock();
  ~spinlock();
  void lock();
  void unlock();

  private:
  // 1 -> locked, 0 -> unlocked. Only changed through atomic_compare_and_swap(), or by the owner in unlock().
  volatile uint32_t locked{0};
};

}  // namespace YesRTOS
