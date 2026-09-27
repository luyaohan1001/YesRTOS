/**
 * @file i3c_concurrent.cpp
 * @brief Two threads repeatedly read constant CCC values (GETPID; GETMWL and GETBCR) from the same I3C target, while
 *        a higher priority thread, woken from the tick hook, reads its DCR on every tick and so preempts them between
 *        queuing a command and taking its response. The bus mutex must keep command/response sequences apart:
 *        interleaved commands would take each other's responses and data, and any value read would change.
 * @note  QEMU completes each command as soon as it is queued; the tick preemption is what puts a second thread on the
 *        controller in the middle of a sequence.
 */
#include "atomic_section.hpp"
#include "dw_i3c.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "semaphore.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static const uint32_t ROUNDS = 20000;
static const uint32_t MIN_PREEMPTIONS = 30;
static DwI3cBus bus(0);
static I3cDevice dev;
static volatile uint32_t finished = 0;
static volatile uint32_t preemptions = 0;
static Semaphore tick(0, 1);

extern "C" void yesrtos_tick_hook(void) {
  tick.release();
}

static void finish() {
  atomic_section a;
  if (++finished == 2) {
    yesrtos_test::print("tick preemptions=");
    yesrtos_test::print_u32(preemptions);
    yesrtos_test::print("\n");
    yesrtos_test::check(preemptions >= MIN_PREEMPTIONS, "too few preemptions to exercise the bus mutex");
    yesrtos_test::pass();
  }
}

static void pid_reader() {
  for (uint32_t round = 0; round < ROUNDS; round++) {
    uint8_t pid[6];
    yesrtos_test::check(bus.ccc_get(I3cCcc::GETPID, dev.dynamic_address, pid, 6) == I3cStatus::OK, "GETPID failed");
    yesrtos_test::check(I3cBus::decode_pid(pid) == dev.pid, "GETPID answer corrupted by another thread");
    PreemptFIFOScheduler::yield();
  }
  finish();
}

static void mwl_bcr_reader() {
  for (uint32_t round = 0; round < ROUNDS; round++) {
    uint8_t mwl[2] = {0, 0}, bcr = 0;
    yesrtos_test::check(bus.ccc_get(I3cCcc::GETMWL, dev.dynamic_address, mwl, 2) == I3cStatus::OK, "GETMWL failed");
    yesrtos_test::check(((mwl[0] << 8) | mwl[1]) == 256, "GETMWL answer corrupted by another thread");
    yesrtos_test::check(bus.ccc_get(I3cCcc::GETBCR, dev.dynamic_address, &bcr, 1) == I3cStatus::OK, "GETBCR failed");
    yesrtos_test::check(bcr == dev.bcr, "GETBCR answer corrupted by another thread");
    PreemptFIFOScheduler::yield();
  }
  finish();
}

static void interrupter() {
  while (1) {
    tick.acquire();
    uint8_t dcr = 0;
    yesrtos_test::check(bus.ccc_get(I3cCcc::GETDCR, dev.dynamic_address, &dcr, 1) == I3cStatus::OK, "GETDCR failed");
    yesrtos_test::check(dcr == dev.dcr, "GETDCR answer corrupted by another thread");
    preemptions = preemptions + 1;
  }
}

static void setup() {
  size_t found = 0;
  yesrtos_test::check(bus.assign_dynamic_addresses(&dev, 1, found) == I3cStatus::OK && found == 1, "target not found");
  static Thread a(0, pid_reader, 1), b(1, mwl_bcr_reader, 1), h(2, interrupter, 0);
  PreemptFIFOScheduler::add_thread(&h);
  PreemptFIFOScheduler::add_thread(&a);
  PreemptFIFOScheduler::add_thread(&b);
}

int main() {
  static Thread s(9, setup, 1);
  PreemptFIFOScheduler::add_thread(&s);
  PreemptFIFOScheduler::start();
  return 0;
}
