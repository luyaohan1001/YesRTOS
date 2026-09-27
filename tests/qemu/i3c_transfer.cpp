/**
 * @file i3c_transfer.cpp
 * @brief Private transfers on the DesignWare I3C controller with one mock target (a 256 byte buffer whose pointer
 *        advances with every byte written or read and resets on STOP): write then read back, a repeated START
 *        write_read (no STOP, so the read continues after the written bytes), NACK from an absent address and a
 *        direct CCC to it, each followed by a successful transfer (the controller halts on errors and must resume).
 */
#include "dw_i3c.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static void run() {
  static DwI3cBus bus(0);
  I3cDevice dev;
  size_t found = 0;
  yesrtos_test::check(bus.assign_dynamic_addresses(&dev, 1, found) == I3cStatus::OK && found == 1, "no target found");
  const uint8_t addr = dev.dynamic_address;

  const uint8_t data[9] = {0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87, 0x98};  // 9: not a multiple of 4
  yesrtos_test::check(bus.write(addr, data, sizeof(data)) == I3cStatus::OK, "private write failed");
  uint8_t back[9] = {0};
  yesrtos_test::check(bus.read(addr, back, sizeof(back)) == I3cStatus::OK, "private read failed");
  for (uint32_t i = 0; i < sizeof(data); i++) yesrtos_test::check(back[i] == data[i], "read back differs");

  // Repeated START: writing 2 bytes moves the pointer to 2, the read continues from there.
  const uint8_t two[2] = {0xa0, 0xa1};
  uint8_t next[3] = {0};
  yesrtos_test::check(bus.write_read(addr, two, 2, next, 3) == I3cStatus::OK, "write_read failed");
  yesrtos_test::check(next[0] == data[2] && next[1] == data[3] && next[2] == data[4], "no repeated START between write and read");
  yesrtos_test::check(bus.read(addr, back, 2) == I3cStatus::OK && back[0] == 0xa0 && back[1] == 0xa1, "write_read did not write");

  const uint8_t one = 0x5a;
  yesrtos_test::check(bus.write(0x30, &one, 1) == I3cStatus::NACK, "absent address did not NACK");
  yesrtos_test::check(bus.read(addr, back, 1) == I3cStatus::OK && back[0] == 0xa0, "controller did not recover after a NACK");
  uint8_t pid[6];
  yesrtos_test::check(bus.ccc_get(I3cCcc::GETPID, 0x31, pid, 6) == I3cStatus::NACK, "direct CCC to an absent address did not NACK");
  yesrtos_test::check(bus.write(addr, data, 4) == I3cStatus::OK, "controller did not recover after a CCC NACK");
  yesrtos_test::pass();
}

int main() {
  static Thread t(0, run);
  PreemptFIFOScheduler::add_thread(&t);
  PreemptFIFOScheduler::start();
  return 0;
}
