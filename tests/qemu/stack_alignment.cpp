/**
 * @file stack_alignment.cpp
 * @brief AAPCS requires SP to be 8-byte aligned at every public function entry. Each thread checks the stack pointer
 *        it starts with, i.e. the one handed over by the exception return from its initial stack frame.
 * @note  Threads are created with different stack placements (odd number of words of padding between them), so an
 *        unaligned thread stack shows up regardless of where the linker puts the Thread objects.
 */
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static const uint32_t THREADS = 4;
static volatile uint32_t checked = 0;
static volatile uint32_t misaligned = 0;

// Naked entry: read SP before any prologue can adjust it, then continue in C.
extern "C" void check_entry_sp(uint32_t sp);
extern "C" void __attribute__((naked)) aligned_entry() {
  __asm volatile(
    "mov r0, sp       \n"
    "b   check_entry_sp\n");
}

extern "C" void check_entry_sp(uint32_t sp) {
  if (sp % 8 != 0) misaligned = misaligned + 1;
  checked = checked + 1;
  if (checked == THREADS) {
    yesrtos_test::print("misaligned=");
    yesrtos_test::print_u32(misaligned);
    yesrtos_test::print("\n");
    yesrtos_test::check(misaligned == 0, "thread started with an SP that is not 8-byte aligned");
    yesrtos_test::pass();
  }
}  // returns to the thread's initial LR (yesrtos_thread_exit): aligned_entry branched here without linking

// One padding word between the threads shifts every other Thread object by 4 bytes.
struct PaddedThread {
  Thread thread;
  uint32_t pad;
  PaddedThread(uint32_t id) : thread(id, aligned_entry) {}
};

int main() {
  static PaddedThread t0(0), t1(1), t2(2), t3(3);
  PreemptFIFOScheduler::add_thread(&t0.thread);
  PreemptFIFOScheduler::add_thread(&t1.thread);
  PreemptFIFOScheduler::add_thread(&t2.thread);
  PreemptFIFOScheduler::add_thread(&t3.thread);
  PreemptFIFOScheduler::start();
  return 0;
}
