/**
 * @file test_support.hpp
 * @brief Helpers for QEMU regression tests: print over the trace backend and report the result as QEMU's exit status.
 * @note Needs QEMU started with "-semihosting-config enable=on,target=native" (see tests/qemu/CMakeLists.txt).
 *       SYS_EXIT with ADP_Stopped_ApplicationExit makes QEMU exit with status 0, any other reason with status 1.
 *       A test that faults ends up in Default_Handler's infinite loop and is failed by the ctest timeout.
 */
#pragma once

#include <cstdint>

#include "baremetal_api.h"

namespace yesrtos_test {

static constexpr uint32_t SYS_EXIT = 0x18;
static constexpr uint32_t ADP_STOPPED_APPLICATION_EXIT = 0x20026;
static constexpr uint32_t ADP_STOPPED_RUNTIME_ERROR = 0x20023;

[[noreturn]] inline void semihost_exit(uint32_t reason) {
  register uint32_t op __asm("r0") = SYS_EXIT;
  register uint32_t arg __asm("r1") = reason;
  __asm volatile("bkpt 0xab" : : "r"(op), "r"(arg) : "memory");
  while (1) {
  }
}

inline void print(const char* str) {
  itm_trace(str);
}

inline void print_u32(uint32_t value) {
  char buf[11];
  int i = sizeof(buf) - 1;
  buf[i] = '\0';
  do {
    buf[--i] = static_cast<char>('0' + value % 10);
    value /= 10;
  } while (value);
  itm_trace(&buf[i]);
}

// pass() and fail() disable exceptions first, so no other thread can interleave its own result with the report.
[[noreturn]] inline void pass() {
  disable_exception();
  print("PASS\n");
  semihost_exit(ADP_STOPPED_APPLICATION_EXIT);
}

[[noreturn]] inline void fail(const char* why) {
  disable_exception();
  print("FAIL: ");
  print(why);
  print("\n");
  semihost_exit(ADP_STOPPED_RUNTIME_ERROR);
}

inline void check(bool condition, const char* what) {
  if (!condition) fail(what);
}

}  // namespace yesrtos_test
