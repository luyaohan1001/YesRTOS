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

#include <scheduler.hpp>
#include <thread.hpp>

// SysTick memory mapped registers and bits
#define SYST_CSR (*((volatile uint32_t *)0xe000e010))  // SysTick Control and Status Register
#define SYST_RVR (*((volatile uint32_t *)0xe000e014))  // SysTick Reload Value Register
#define SYST_CVR (*((volatile uint32_t *)0xe000e018))  // SysTick Current Value Register
#define SYST_CLKSOURCE_BIT (1UL << 2UL)
#define SYST_TICKINT_BIT (1UL << 1UL)
#define SYST_ENABLE_BIT (1UL << 0UL)

// SysTick register reload count calculation.
#define SYSTICK_RELOAD_CNT ((CPU_CLK_FREQ_HZ / TIMESLICE_FREQ_HZ) - 1UL)

// Interrupt memory mapped registers
#define ICSR (*((volatile uint32_t *)0xe000ed04))  // Interrupt control and status register
#define PENDSVSET_BIT (1UL << 28UL)                // sets the PendSV exception as pending.

// Declare scheduler instance. This is a global instance.
YesRTOS::PreemptiveScheduler sched;

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

/**
 * @brief SysTick exception handler.
 *        On each entry it saves context for current running thread, and load context to next thread pointed by scheduler.
 * @note stmdb, pseudo instruction: https://developer.arm.com/documentation/ddi0403/d/Application-Level-Architecture/Instruction-Details/Alphabetical-list-of-ARMv7-M-Thumb-instructions/STMDB--STMFD
 * @note stmia, pseudo instruction:
 * https://developer.arm.com/documentation/ddi0403/d/Application-Level-Architecture/Instruction-Details/Alphabetical-list-of-ARMv7-M-Thumb-instructions/STM--STMIA--STMEA
 */
extern "C" {
void SysTick_Handler() {
  // Saving context of current thread.
  // $r1 = process stack pointer
  __asm volatile("mrs r1, psp");
  // store multiple decrement before. (psp-=4;*psp=r14; psp-=4;*psp=r11; psp-=4;*psp=r10... psp-=4,*psp=r4)
  __asm volatile("stmdb r1!, {r4-r11, r14}");
  // $r0 = sched.pp_active_thread_stk
  __asm volatile("mov r0, %0" : : "r"(sched.pp_active_thread_stk) : "memory");  // $r0 = pointer to current active thread's stack pointer
  // *$r0 = r1 ==> *sched.pp_active_thread_stk = $r1 = psp (storing psp back to thread context)
  __asm volatile("str r1, [r0]");

  // schduler updates pp_active_thread_stk to next thread.
  sched.schedule_next();

  // Restore context of next thread.
  // $r0 = sched.pp_active_thread_stk
  __asm volatile("mov r0, %0" : : "r"(sched.pp_active_thread_stk) : "memory");
  // $r0 = *$r0 = *sched.pp_active_thread_stk which gives psp
  __asm volatile("ldr r0, [r0]");
  // load multiple, increment after. (r4=*psp;psp+=4; r5=*psp;psp+=4; r6=*psp;psp+=4;... r14=*psp;psp+=4;)
  __asm volatile("ldmia r0!, {r4-r11, r14}");
  // psp = r0 ==> update process stack pointer to top of the stack
  __asm volatile("msr psp, r0");
}
}

/**
 * @brief Initialize stack for any running thread for the first time.
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
void init_stack_armv7m(uint32_t **pp_stk, uint32_t *routine_ptr) {
  (*pp_stk)--;              // walk stack pointer
  (**pp_stk) = 0x01000000;  // xPSR

  (*pp_stk)--;                                                    // walk stack pointer
  (**pp_stk) = ((uint32_t)(uintptr_t)routine_ptr) & 0xfffffffeUL; /* PC */

  (*pp_stk)--;  // walk stack pointer, reserve for function return address (LR)

  (*pp_stk) -= 5;  // walk stack pointer, reserve for R0-R3, R12

  (*pp_stk)--;              // walk stack pointer
  (**pp_stk) = 0xfffffffd;  // return to handler mode using psp.

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
 * @brief SuperVisor Call Handler
 * @note Exception handler by which the core services a SVC. Handler mode.
 */
extern "C" {
void SVC_Handler(void) {
  __asm volatile("mov r0, %0" : : "r"(sched.pp_active_thread_stk) : "memory");
  __asm volatile("ldr r0, [r0]");
  __asm volatile("ldmia r0!, {r4-r11, r14}");
  __asm volatile("msr psp, r0");  // Point process stack pointer to top of the stack after popping.
  __asm volatile("bx r14");       // r14 expected 0xfffffffd (indicate exiting handler mode to thread mode using Procees Stack Pointer)
}
}
