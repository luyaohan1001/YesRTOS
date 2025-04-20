#include <cstddef>  // size_t

/**
 * @brief Load from memory and simultaneously mark the accessed memory location as being exclusively held by the current thread.
 * @param[in] p_mem Memory address to perform load.
 * @param[out] size_t Data read from memory.
 * @note LDREX instruction https://developer.arm.com/documentation/dui0489/i/arm-and-thumb-instructions/ldrex
 * @note pseudo code: $r0 = *p_mem
 *                    mem_read = $r0
 */
extern "C" {
  size_t exclusive_load(bool *const p_mem) {
  volatile size_t mem_read;
  __asm volatile("ldrex %0, [%1]" : "=r"(mem_read) : "r"(p_mem) : "memory");
  return mem_read;
  }
}

/**
 * @brief Conditional store to memory. If other thread already attempted, then exclusiveness already invalidated, strex fails.
 * @param[in] p_mem Memory address to perform store.
 * @param[in] new_val Value to store.
 * @param[out] bool Status. If @p_mem exclusive flag is still set, store success and return status 0. Else, return status 1.
 * @note STREX instruction https://developer.arm.com/documentation/dui0379/e/arm-and-thumb-instructions/strex
 * @note pseudo code:
 *                  if (p_mem's EX FLAG == false) return 1;
 *                  else: *pmem = new_val; return 0;
 * $r0 = *p_mem
 *                    mem_read = $r0
 */
extern "C" {
  bool exclusive_store(bool *const p_mem, size_t new_val) {
  volatile size_t status;
  // %0: output operand to status '=&r': earlyclobber to compiler indicating 'status' filled by instruction itself. As result different register is used for it.
  // %1: input operand, load from new_val into register.
  // %2: input operand, load from p_mem into register, [] indicates deference.
  __asm volatile("strex %0, %1, [%2]" : "=&r"(status) : "r"(new_val), "r"(p_mem) : "memory");
  return status;
  }
}

/**
 * @brief Compare and swap (https://en.wikipedia.org/wiki/Compare-and-swap)
 * @note Leverage LDREX and STREX instruction to achieve atomic swap on memory.
 */
extern "C" {
bool atomic_compare_and_swap(bool *const p_mem, size_t old_val, size_t new_val) {
  // atomic load from memory.
  volatile size_t mem_read = exclusive_load(p_mem);

  // memory already changed?
  if (mem_read != old_val) {
    return true;
  }

  // Not changed, attempt to change.
  volatile bool status = exclusive_store(p_mem, new_val);
  return status;
}
}