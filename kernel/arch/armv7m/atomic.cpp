#include <cstddef>  // size_t

/**
 * @brief compare and swap (https://en.wikipedia.org/wiki/Compare-and-swap)
 * @note Leverage "Load register exclusive and store register exclusive" instruction to achieve atomic swap on memory.
 * @note ldrex and strex assembly document: https://developer.arm.com/documentation/dui0231/b/arm-instruction-reference/arm-memory-access-instructions/ldrex-and-strex
 * @note Assembler Instructions with C expressions Operand doc: https://gcc.gnu.org/onlinedocs/gcc-3.2.3/gcc/Extended-Asm.html
 */
extern "C" {
bool atomic_compare_and_swap(bool *const p_mem, size_t old_val, size_t new_val) {
  // load the lock value
  volatile size_t mem_read;
  volatile size_t status;
  // %0: output operand, store register value into mem_read. %1: input operand, load from p_mem into register, [] indicates address read.
  // pseudo code: ldrex mem_read, [p_mem]
  __asm volatile("ldrex r0, [%0]" ::"r"(p_mem) : "memory");
  // move r0 to mem_read
  __asm volatile("mov %0, r0" : "=r"(mem_read)::"memory");

  // in case of try locking, is the lock already taken?
  if (mem_read != old_val) {
    return true;
  }

  // in case of try locking, did it succeed?
  // %0: output operand, store register value into status. %1: inout operand, load from new_val into register. %2: input operand, load from p_mem into register, [] indicates address read.
  // pseudo code: strex status, new_val, [p_mem]
  // status: 0 := success

  // Below caused infinite loop
  __asm volatile("strex r0, %0, [%1]" : : "r"(new_val), "r"(p_mem) : "memory");
  __asm volatile("str r0, [%0]" : "=r"(status)::"memory");

  // Separate into different instructions
  // __asm volatile("strex r1, %0, [%1]": : "r"(new_val), "r"(p_mem) : "memory");
  // __asm volatile("str r1, [%0]": "=r"(status) :: "memory");

  return status;
}
}