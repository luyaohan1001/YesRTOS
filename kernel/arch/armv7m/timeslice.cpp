/**
 * @file timeslice.cpp
 * @author luyaohan1001 (luyaohan1001@gmail.com)
 * @brief Implementation of systick timer and exception handlers to support timeslice based thread switch.
 * @note This is in C++ file to compile with C++ same as YesRTOS kernel.
 * @note Extern "C" is used for all exception handlers to make sure symbol known to compiler is protected from C++ name mangling.
 * @version 0.1
 * @date 2024-07-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <baremetal_api.h>
#include <config.h>

#include <preempt_fifo_scheduler.hpp>
#include <rr_scheduler.hpp>
#include <thread.hpp>

// SysTick memory mapped registers and bits
#define SYST_CSR (*((volatile uint32_t *)0xe000e010))  // SysTick Control and Status Register
#define SYST_RVR (*((volatile uint32_t *)0xe000e014))  // SysTick Reload Value Register
#define SYST_CVR (*((volatile uint32_t *)0xe000e018))  // SysTick Current Value Register
#define SYST_CLKSOURCE_BIT (1UL << 2UL)
#define SYST_TICKINT_BIT (1UL << 1UL)
#define SYST_ENABLE_BIT (1UL << 0UL)

// SysTick register reload count calculation, systick counts from the reload to zero.
#define SYSTICK_RELOAD_CNT ((CPU_CLK_FREQ_HZ / TIMESLICE_FREQ_HZ) - 1UL)
static_assert(SYSTICK_RELOAD_CNT >  0x0);
static_assert(SYSTICK_RELOAD_CNT <=  0x00FFFFFF);

// Interrupt memory mapped registers
#define ICSR (*((volatile uint32_t *)0xe000ed04))  // Interrupt control and status register
#define PENDSVSET_BIT (1UL << 28UL)                // sets the PendSV exception as pending.
#define SHPR3 (*((volatile uint32_t *)0xe000ed20))  // System Handler Priority Register 3: PendSV [23:16], SysTick [31:24]
#define SHPR3_PENDSV_PRI_LOWEST (0xFFUL << 16UL)
#define SHPR3_SYSTICK_PRI_LOWEST (0xFFUL << 24UL)

/**
 * @brief Give PendSV and SysTick the lowest exception priority.
 * @note  At reset every configurable exception has priority 0, the highest. The context switch (PendSV) must only run
 *        once no other exception is active, otherwise it swaps thread stacks underneath an interrupted handler; the tick
 *        only requests that switch, so it does not need to preempt interrupt handlers either. Writing 0xFF selects the
 *        lowest priority whatever number of priority bits the core implements.
 */
extern "C" {
  void kernel_exception_priority_init(void) {
    SHPR3 |= SHPR3_PENDSV_PRI_LOWEST | SHPR3_SYSTICK_PRI_LOWEST;
  }
}

/**
 * @brief Start systick timer.
 */
extern "C" {
  void systick_clk_init(void) {
    SYST_CSR = SYST_CVR = 0UL;                                             // clear status and current value
    SYST_RVR = SYSTICK_RELOAD_CNT;                                         // set reload value
    SYST_CSR = (SYST_CLKSOURCE_BIT | SYST_TICKINT_BIT | SYST_ENABLE_BIT);  // enable and make count to 0 trigger the SysTick exception.
  }
}

extern "C" {
  void request_context_switch() {
    ICSR |= PENDSVSET_BIT;
  }
}

extern "C" {
  void SysTick_Handler() {
    request_context_switch();
  }
}


/**
 * @brief Save the outgoing thread's stack pointer, pick the next thread and return its stack pointer.
 * @param[in] sp Outgoing thread's PSP, after PendSV_Handler pushed r4-r11 onto it.
 * @return Incoming thread's saved stack pointer, pointing at its r4-r11.
 * @note Plain C function called from PendSV_Handler, so the compiler handles all C++ work with the normal ABI.
 */
extern "C" {
uint32_t* yesrtos_switch_context(uint32_t* sp) {
  YesRTOS::PreemptFIFOScheduler::p_active_thread->stkptr = sp;
  YesRTOS::PreemptFIFOScheduler::schedule_next();
  return (uint32_t*)YesRTOS::PreemptFIFOScheduler::p_active_thread->stkptr;
}
}

/**
 * @brief PendSV exception handler.
 *        On each entry it saves context for current running thread, and load context to next thread pointed by scheduler.
 * @note stmdb, pseudo instruction: https://developer.arm.com/documentation/ddi0403/d/Application-Level-Architecture/Instruction-Details/Alphabetical-list-of-ARMv7-M-Thumb-instructions/STMDB--STMFD
 * @note stmia, pseudo instruction:
 * https://developer.arm.com/documentation/ddi0403/d/Application-Level-Architecture/Instruction-Details/Alphabetical-list-of-ARMv7-M-Thumb-instructions/STM--STMIA--STMEA
 * @note According to AAPCS, R0, R1, R2, R3, R12, LR, PC, xPSR are automatically saved by hardware during exception, thus they are allowed to overwrite immediately entering this handler function.
 * @note __attribute__((naked)) is extremely important to compile as pure assembly function without C prologue.
 * @note A naked function may only contain basic asm (no operands): with operands the compiler picks registers on its
 *       own, which could clobber r0/r1 or r4-r11 between statements, and it cannot spill anything since there is no
 *       frame. The C++ part is therefore a normal function, yesrtos_switch_context(), taking and returning the stack
 *       pointer in r0.
 * @note The push/pop keeps MSP 8-byte aligned across the call, as AAPCS requires; r3 is only padding.
 */
extern "C" {
void __attribute__((naked)) PendSV_Handler() {
  __asm volatile(
    "mrs    r0, psp                 \n"  // r0 = outgoing thread's stack pointer.
    "isb                            \n"
    "stmdb  r0!, {r4-r11}           \n"  // save the registers the hardware did not stack (psp-=4; *psp=r11; ... r4).
    "push   {r3, lr}                \n"  // protect EXC_RETURN across the call; r3 pads the push to 8 bytes.
    "bl     yesrtos_switch_context  \n"  // r0 = incoming thread's stack pointer.
    "pop    {r3, lr}                \n"
    "ldmia  r0!, {r4-r11}           \n"  // restore the incoming thread's r4-r11 (r4=*psp; psp+=4; ...).
    "msr    psp, r0                 \n"  // the exception return pops the rest of its context from here.
    "isb                            \n"
    "bx     lr                      \n");
}
}

/**
 * @brief Initialize stack for any running thread for the first time, it is MIMICKING AN EXCEPTION STACK!
 * @param [in] pp_stk Pointer to stack pointer.
                      (*pp_stk)--; to grow stack
 *                    (**pp_stk)=0xBEEF; to assign value to stack location.
 *                    On thread object creation, this is expected to be the same value as bottom of the stack.
 * @param [in] routine_ptr Function routine to be executed for this thread.
 * @note  When init_stack_armv7m() is finished, the stack contains necessary context for it to be scheduled for the first time.
 *        The pp_stk will grow (decrements in this case a full-descending stack for this arch) as values are written.
 * @note Stack content when this function is finished:
 *
 */

extern "C" {
void init_stack_armv7m(volatile uint32_t **pp_stk, uint32_t *routine_ptr) {
  extern void yesrtos_thread_exit(void);

  // Exception-entry HW saved registers. Mimic the stack context of an exception.
  (*pp_stk)--;              // full descending stack, decrement 1 to point to first empty position.
  (**pp_stk) = 0x01000000;  // xPSR

  (*pp_stk)--;                                                    // full descending stack, decrement 1 to point to first empty position.
  (**pp_stk) = ((uint32_t)(uintptr_t)routine_ptr) & 0xfffffffeUL; /* PC */

  (*pp_stk)--;  // walk stack pointer, reserve for function return address (LR)
  (**pp_stk) = (uint32_t)(uintptr_t)&yesrtos_thread_exit;  // a routine that returns ends its thread there.

  (*pp_stk) -= 5;  // walk stack pointer, reserve for machine saved registers including R0, R1, R2, R3, R12.

  // Software saved registers.
  (*pp_stk) -= 8;  // walk stack pointer, reserve for R11-R4
}
}

/**
 * @brief Make supervisor call to trigger SVC_Handler.
 * @note  This function is called only ONCE, i.o.w, when the scheduler start first thread for the first time.
 *        So that SVC_Handler could be triggered to return to *Thread Mode* using *Process Stack Pointer (PSP)*.
 */
extern "C" {
void start_first_task(void) {
  __asm volatile("svc 0"); /* System call to start first thread. */
}
}
/**
 * @brief Stack pointer of the first thread to run.
 * @return Saved stack pointer of the active thread, pointing at its initial r4-r11.
 */
extern "C" {
uint32_t* yesrtos_first_context(void) {
  return (uint32_t*)YesRTOS::PreemptFIFOScheduler::p_active_thread->stkptr;
}
}

/**
 * @brief SuperVisor Call Handler
 * @note Exception handler by which the core services a SVC. Handler mode.
 * @note Basic asm only, for the same reason as PendSV_Handler. MSP is 8-byte aligned on exception entry, so the call
 *       needs no padding; LR is overwritten below anyway.
 */
extern "C" {
void __attribute__((naked)) SVC_Handler(void) {
  __asm volatile(
    "bl     yesrtos_first_context  \n"  // r0 = first thread's stack pointer.
    "ldmia  r0!, {r4-r11}          \n"
    "msr    psp, r0                \n"  // Point process stack pointer to top of the stack after popping.
    "isb                           \n"
    "mvn    lr, #2                 \n"  // lr = 0xFFFFFFFD: return to Thread mode using the Process Stack Pointer.
    "bx     lr                     \n");
}
}
