/**
 * @file context.s
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Context switch implementation specific to ARM-V7 Cortex-M Architecture.
 * @version 1.0
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

// directives specifying architecture and assembler options
  .syntax unified
  .cpu cortex-m7
  .fpu softvfp
  .thumb

// Pointer to scheduler stack for context saving and restoring.
.extern sched_stk_ptr

// Yield thread. Thus function assumes one argument (passed to $r0) containing pointer to the thread.
.global yield_thread
.type  yield_thread, %function
yield_thread:
  stmia r0!, {r4-r11, r14}
  bl load_sched_context

// Save scheduler context.
.global save_sched_context
.type  save_sched_context, %function
save_sched_context:
  ldr r0, =sched_stk_ptr
  ldr r0, [r0]
  stmia r0!, {r4-r11, r14}
  bx lr

// Load scheduler context.
.type  load_sched_context, %function
load_sched_context:
  ldr r0, =sched_stk_ptr
  ldr r0, [r0]
  ldmia r0!, {r4-r11, r14}
  bx lr