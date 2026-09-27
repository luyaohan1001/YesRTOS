# CS-017: Interrupt vector table without external interrupts

**Category:** Undefined behaviour (interrupt handling)  **Status:** Fixed in `b983f0e`  **Test:** `tests/qemu/external_interrupt`

## Hazard
All external interrupt entries of the startup vector table were commented out, leaving only the 16 system exceptions.
Any enabled interrupt made the core load its handler address from whatever followed the table and jump there. Nothing
used interrupts yet, so it stayed hidden until the first interrupt-driven code (a semaphore released from an ISR).

## Example
`kernel/arch/armv7m/startup_stm32f767xx.s` (before):
```asm
.word  SysTick_Handler

/* External Interrupts */
// .word     WWDG_IRQHandler                   /* Window WatchDog */
// .word     PVD_IRQHandler                    /* PVD through EXTI Line detection */
...                                           (all 110 entries)
```

## Fix
Generate numbered, weak `IRQ<n>_Handler` entries aliased to `Default_Handler`, with the count set per board
(`YESRTOS_NUM_IRQS`: 110 STM32F76x, 96 QEMU netduinoplus2, 272 QEMU ast1030):
```asm
.rept YESRTOS_NUM_IRQS
  irq_vector %irq_number        // .word IRQ<n>_Handler
  .set irq_number, irq_number + 1
.endr
```

## How it was found
The semaphore ISR test HardFaulted at the first software-triggered IRQ; gdb showed a jump to `0x4b044802`, and the IRQ
handler symbol was missing from the image. Measuring which IRQs QEMU delivers also showed netduinoplus2 handles 0..95
only, although its NVIC reports 112.

## Design rule
Every interrupt the hardware can raise needs a vector entry, even if it only points at a default handler; size the
table from the device, and verify with an interrupt test before relying on interrupts.
