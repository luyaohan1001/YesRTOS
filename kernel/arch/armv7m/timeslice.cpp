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
 * @note According to AAPCS, R0, R1, R2, R3, R12, LR, PC, xPSR are automatically saved by hardware during exception, thus they are allowed to overwrite immediately entering this handler function.
 * @note Assembler Instructions with C Expression Operands: https://gcc.gnu.org/onlinedocs/gcc-3.2.3/gcc/Extended-Asm.html
 */
extern "C" {
void SysTick_Handler() {
  ICSR |= PENDSVSET_BIT;
}
}

/**
 * @brief
 * @note __attribute__((naked)) is extremely important to compile as pure assembly function without C prologue
 */
extern "C" {
void __attribute__((naked)) PendSV_Handler() {
  // Saving context of current thread.
  // $r1 = process stack pointer
  __asm volatile("mrs r1, psp" ::: "r1");
  __asm volatile("isb");
  // store multiple decrement before. (psp-=4;*psp=r14; psp-=4;*psp=r11; psp-=4;*psp=r10... psp-=4,*psp=r4)
  __asm volatile("stmdb r1!, {r4-r11}" ::: "r1");
  // $r0 = sched.p_active_thread
  // no output operand. %0: input operand, load p_active_thread into register, memory clobber.
  __asm volatile("mov r0, %0" : : "r"(&YesRTOS::PreemptFIFOScheduler::p_active_thread->stkptr) : "r0", "memory");  // $r0 = pointer to current active thread's stack pointer
  // *$r0 = r1 ==> *sched.p_active_thread = $r1 = psp (storing psp back to thread context)
  __asm volatile("str r1, [r0]" ::: "r0", "r1");
  __asm volatile("push {r4-r11, lr} ");

  // scheduler updates p_active_thread to next thread.
  YesRTOS::PreemptFIFOScheduler::schedule_next();

  __asm volatile("pop {r4-r11, lr} ");

  // Restore context of next thread.
  // $r0 = sched.p_active_thread
  // no output operand. %0: input operand, load p_active_thread into register, memory clobber.
  __asm volatile("mov r0, %0" : : "r"(&YesRTOS::PreemptFIFOScheduler::p_active_thread->stkptr) : "r0", "memory");
  // $r0 = *$r0 = *sched.p_active_thread which gives psp
  __asm volatile("ldr r0, [r0]" ::: "r0");
  // load multiple, increment after. (r4=*psp;psp+=4; r5=*psp;psp+=4; r6=*psp;psp+=4;... r14=*psp;psp+=4;)
  __asm volatile("ldmia r0!, {r4-r11}" ::: "r0");
  // psp = r0 ==> update process stack pointer to top of the stack
  __asm volatile("msr psp, r0" ::: "r0");
  __asm volatile("isb");
  __asm volatile("bx lr");
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
  // Exception-entry HW saved registers. Mimic the stack context of an exception.
  (*pp_stk)--;              // full descending stack, decrement 1 to point to first empty position.
  (**pp_stk) = 0x01000000;  // xPSR

  (*pp_stk)--;                                                    // full descending stack, decrement 1 to point to first empty position.
  (**pp_stk) = ((uint32_t)(uintptr_t)routine_ptr) & 0xfffffffeUL; /* PC */

  (*pp_stk)--;  // walk stack pointer, reserve for function return address (LR)
  (**pp_stk) = 0xDEADBEEF;

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
 * @brief SuperVisor Call Handler
 * @note Exception handler by which the core services a SVC. Handler mode.
 */
extern "C" {
void __attribute__((naked)) SVC_Handler(void) {
  __asm volatile("mov r0, %0" : : "r"(&YesRTOS::PreemptFIFOScheduler::p_active_thread->stkptr) : "memory");
  __asm volatile("ldr r0, [r0]");
  __asm volatile("ldmia r0!, {r4-r11}");
  __asm volatile("msr psp, r0");          // Point process stack pointer to top of the stack after popping.
  __asm volatile("ldr lr, =0xFFFFFFFD");  // r14 expected 0xfffffffd (indicate exiting handler mode to thread mode using Procees Stack Pointer)
  __asm volatile("bx lr");                // r14 expected 0xfffffffd (indicate exiting handler mode to thread mode using Procees Stack Pointer)
}
}
