/**
 * @file atomic_section.hpp
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief RAII-based class for managing atomic (exception-disabled) sections.
 * @note  This class disables interrupts (or exceptions) upon construction and restores them upon destruction, ensuring atomic execution within the scope where the object exists.
 * @note  Sections may nest: the destructor restores the exception mask saved by the constructor instead of unconditionally
 *        re-enabling exceptions, so an inner section never re-enables exceptions inside an outer one.
 * It is intended for use where critical sections are required for thread-safe or interrupt-safe operations.
 *
 * Copying is explicitly disallowed to avoid unintended re-enabling of exceptions.
 * @version 0.0
 * @date 2025-04-21
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <cstdint>

#if defined(ARMV7M)
#include "baremetal_api.h"
#endif

class atomic_section {
  public:
  atomic_section();
  ~atomic_section();

  private:
  // PRIMASK before this section disabled exceptions.
  uint32_t saved_primask;

  public:

  // disallow copying.
  atomic_section(const atomic_section&)=delete;
  // disallow copy assignment.
  atomic_section&  operator=(const atomic_section&)=delete;
};