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
  // uint32_t atomic_val;
  std::atomic<bool> locked;
};

}  // namespace YesRTOS
