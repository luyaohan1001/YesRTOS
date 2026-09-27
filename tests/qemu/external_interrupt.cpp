/**
 * @file external_interrupt.cpp
 * @brief External interrupts reach the numbered IRQ<n>_Handler of the vector table. IRQs 0 and 90 exist on every
 *        board (QEMU's netduinoplus2 delivers IRQs 0..95 only); both are made pending by software through the NVIC, so no peripheral is involved.
 */
#include "test_support.hpp"

#define NVIC_ISER(n) (*((volatile uint32_t*)(0xe000e100 + 4 * (n))))  // Interrupt Set-Enable Registers
#define NVIC_ISPR(n) (*((volatile uint32_t*)(0xe000e200 + 4 * (n))))  // Interrupt Set-Pending Registers

static volatile uint32_t irq0_count = 0;
static volatile uint32_t irq90_count = 0;

extern "C" void IRQ0_Handler(void) {
  irq0_count = irq0_count + 1;
}

extern "C" void IRQ90_Handler(void) {
  irq90_count = irq90_count + 1;
}

static void trigger(uint32_t irq) {
  NVIC_ISER(irq / 32) = 1u << (irq % 32);
  NVIC_ISPR(irq / 32) = 1u << (irq % 32);
  __asm volatile("dsb \n isb" ::: "memory");
}

int main() {
  itm_initialize();
  trigger(0);
  trigger(90);
  yesrtos_test::check(irq0_count == 1, "IRQ 0 did not reach IRQ0_Handler");
  yesrtos_test::check(irq90_count == 1, "IRQ 90 did not reach IRQ90_Handler");
  yesrtos_test::pass();
}
