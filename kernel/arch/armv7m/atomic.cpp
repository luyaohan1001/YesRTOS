#include <cstddef> // size_t

/**
 * @brief compare and swap (https://en.wikipedia.org/wiki/Compare-and-swap)
 * @note Leverage "Load register exclusive and store register exclusive" instruction to achieve atomic swap on memory.
 * @note ldrex and strex assembly document: https://developer.arm.com/documentation/dui0231/b/arm-instruction-reference/arm-memory-access-instructions/ldrex-and-strex
 * @note Assembler Instructions with C expressions Operand doc: https://gcc.gnu.org/onlinedocs/gcc-3.2.3/gcc/Extended-Asm.html
*/
extern "C" {
  bool compare_and_swap(size_t* addr, size_t old_val, size_t new_val) {
    // load the lock value
    size_t addr_val;
    // %0: output operand, store register value into addr_val. %1: input operand, load from addr into register, [] indicates address read.
    // pseudo code: ldrex addr_val, [addr]
    __asm volatile("ldrex %0, [%1]" : "=r"(addr_val) : "r"(addr) : "memory");

    // is the lock already taken?
    if (addr_val != old_val) {
      return true;
    }

    // did try locking succeed?
    size_t status;
    // %0: output operand, store register value into status. %1: inout operand, load from new_val into register. %2: input operand, load from addr into register, [] indicates address read.
    // pseudo code: strex status, new_val, [addr]
    __asm volatile("strex %0, %1, [%2]": "=r"(status) : "r"(new_val), "r"(addr) : "memory");

    return status;
  }
}