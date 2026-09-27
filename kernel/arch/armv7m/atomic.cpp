#include <cstddef>  // size_t
#include <cstdint>

typedef enum {
  SUCCESS = 0,
  FAIL
} eResult_t;

/**
 * @brief Compare and swap (https://en.wikipedia.org/wiki/Compare-and-swap)
 *        Atomically: if (*p_mem == old_val) { *p_mem = new_val; return true; } else { return false; }
 * @param[in] p_mem Word aligned memory address to update.
 * @param[in] old_val Value expected at p_mem.
 * @param[in] new_val Value to store if p_mem still holds old_val.
 * @return true if new_val was stored, false if p_mem did not hold old_val.
 * @note LDREX marks a memory address as "exclusive" for the current thread, and STREX stores a new value only if the exclusive mark is still valid.
 *       https://developer.arm.com/documentation/dui0489/i/arm-and-thumb-instructions/ldrex
 *       https://developer.arm.com/documentation/dui0379/e/arm-and-thumb-instructions/strex
 * @note Strong CAS: an exception (interrupt, context switch) between LDREX and STREX clears the exclusive mark and makes STREX fail
 *       even though the value never changed. That case is retried here, so false always means the value differed.
 * @note LDREX and STREX live in one asm block, so the compiler cannot place other memory accesses (stack spills, function
 *       return) between them, and no exclusive state is left open on return (CLREX on mismatch).
 * @note Barriers: DMB after a successful store gives acquire semantics to a lock taken with this CAS.
 */
extern "C" {
bool atomic_compare_and_swap(volatile uint32_t *p_mem, uint32_t old_val, uint32_t new_val) {
  uint32_t loaded;
  uint32_t status;

  __asm volatile(
    "1: ldrex   %[loaded], [%[mem]]            \n"  // loaded = *p_mem, mark p_mem exclusive.
    "   cmp     %[loaded], %[expected]         \n"
    "   bne     2f                             \n"  // value differs: give up.
    "   strex   %[status], %[desired], [%[mem]]\n"  // *p_mem = new_val if still exclusive; status = 0 on success, 1 on failure.
    "   cmp     %[status], #0                  \n"
    "   bne     1b                             \n"  // exclusive mark lost, value may still match: retry.
    "   dmb                                    \n"
    "   b       3f                             \n"
    "2: clrex                                  \n"  // drop the exclusive mark taken by LDREX.
    "3:                                        \n"
    : [loaded] "=&r"(loaded),                       // '&' = early clobber: written before all inputs are consumed,
      [status] "=&r"(status)                        //       so must not share a register with p_mem / old_val / new_val.
    : [mem] "r"(p_mem),
      [expected] "r"(old_val),
      [desired] "r"(new_val)
    : "cc", "memory"                                // flags are modified by CMP; memory clobber prevents reordering around the CAS.
  );

  return loaded == old_val;
}
}

/**
 * @brief Disable exception.
 * @note CPSID, Change Processor State Interrupt Disable.
 * @note When issuing CPSID the effect is not immediate for instructions in pipeline. Inject barrier to enforce any instructions after barrier to recognize new system state.
 *
 *  From the CPU pipeline from cycles it looks like below such as:
 *       F: INSTR1 D: INSTR2 E: INSTR3 ...
 *       F: CPSID  D: INSTR1 E: INSTR2 ...
 *       F: ISB    D: CPSID  E: INSTR1 ...
 *       F: ISB    D: ISB    E: CPSID  ...
 *       F: ISB    D: ISB    E: ISB    ...
 *       F: INSTR4 D: ISB    E: ISB    ...
 *       ......
 *       With F = FETCH D = DECODE, E = EXECUTE.
 */
extern "C" {
void disable_exception() {
  // disable exception, set PRIMASK to 1.
  __asm volatile("cpsid i" ::: "memory");
  // flush instruction pipeline.
  __asm volatile("isb");
  // synchronize memory load/store.
  __asm volatile("dsb");
}
}

/**
 * @brief Enable exception.
 * @note CPSIE, Change Processor State Interrupt Enable.
 * @note Counterpart to void disable_exception().
 */
extern "C" {
void enable_exception() {
  __asm volatile("cpsie i" ::: "memory");
  __asm volatile("isb");
  __asm volatile("dsb");
}
}

/**
 * @brief Disable exception and return the previous exception mask, for critical sections that may nest.
 * @return Previous PRIMASK value (1 if exceptions were already disabled, 0 otherwise), to hand to restore_exception().
 */
extern "C" {
uint32_t save_and_disable_exception() {
  uint32_t primask;
  __asm volatile("mrs %0, primask" : "=r"(primask) :: "memory");
  disable_exception();
  return primask;
}
}

/**
 * @brief Whether the caller runs in an exception handler (IPSR holds the active exception number, 0 in Thread mode).
 *        Blocking kernel calls use it to refuse being called from interrupt handlers.
 */
extern "C" {
bool in_exception_handler() {
  uint32_t ipsr;
  __asm volatile("mrs %0, ipsr" : "=r"(ipsr));
  return ipsr != 0;
}
}

/**
 * @brief Restore the exception mask saved by save_and_disable_exception().
 * @param[in] primask Previous PRIMASK value. Exceptions are only re-enabled if they were enabled before the matching save.
 * @note An exception made pending inside the critical section (e.g. PendSV) is taken right after this call when it re-enables.
 */
extern "C" {
void restore_exception(uint32_t primask) {
  __asm volatile("msr primask, %0" :: "r"(primask) : "memory");
  __asm volatile("isb");
}
}