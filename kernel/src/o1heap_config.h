/**
 * @file o1heap_config.h
 * @brief Build configuration for the vendored o1heap (third_party/o1heap), passed in via O1HEAP_CONFIG_HEADER.
 */
#pragma once

// A failed o1heap self-check means the heap metadata is corrupted. Stop right there with a fault (UDF -> HardFault)
// instead of newlib's assert(), which would pull in stdio and syscalls this bare-metal kernel does not provide.
#define O1HEAP_ASSERT(x) \
  do {                   \
    if (!(x)) {          \
      __builtin_trap();  \
    }                    \
  } while (0)
