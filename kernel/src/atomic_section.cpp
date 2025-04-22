#include "atomic_section.hpp"

#if defined(ARCH_ARMV7M)
atomic_section::atomic_section() {
    disable_exception();
}

atomic_section::~atomic_section() {
  enable_exception();
}
#endif