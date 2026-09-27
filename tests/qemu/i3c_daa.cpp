/**
 * @file i3c_daa.cpp
 * @brief Dynamic address assignment on the DesignWare I3C controllers of QEMU ast1030-evb: a mock target on each of
 *        controllers 0 and 1 (see QEMU_ARGS_i3c_daa in CMakeLists.txt) and none on controller 2. Each target gets an
 *        address and is identified (PID, BCR, DCR); GETMWL reads the mock target's buffer size; the empty bus reports
 *        no target.
 * @note  QEMU's model does not arbitrate between several unaddressed targets during ENTDAA, so each bus carries one.
 * @note  QEMU's I3C core sends the PID least significant byte first (hw/i3c/core.c), while MIPI I3C sends it most
 *        significant byte first, which the driver follows. So on QEMU the PID reads back byte-reversed.
 */
#include "dw_i3c.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static uint64_t byte_reversed_48(uint64_t v) {
  uint64_t r = 0;
  for (int i = 0; i < 6; i++) r = (r << 8) | ((v >> (8 * i)) & 0xff);
  return r;
}

static void check_bus(DwI3cBus& bus, uint64_t pid, uint8_t bcr, uint8_t dcr) {
  I3cDevice devices[4];
  size_t found = 0;
  yesrtos_test::check(bus.assign_dynamic_addresses(devices, 4, found) == I3cStatus::OK, "dynamic address assignment failed");
  yesrtos_test::check(found == 1, "expected one target");
  const I3cDevice& dev = devices[0];
  yesrtos_test::check(dev.pid == byte_reversed_48(pid), "PID wrong");
  yesrtos_test::check(dev.bcr == bcr && dev.dcr == dcr, "BCR/DCR wrong");

  uint8_t mwl[2] = {0, 0};
  yesrtos_test::check(bus.ccc_get(I3cCcc::GETMWL, dev.dynamic_address, mwl, 2) == I3cStatus::OK, "GETMWL failed");
  yesrtos_test::check(((mwl[0] << 8) | mwl[1]) == 256, "GETMWL is not the mock target's 256 byte buffer");
  const uint8_t data[3] = {7, 8, 9};
  uint8_t back[3] = {0};
  yesrtos_test::check(bus.write(dev.dynamic_address, data, 3) == I3cStatus::OK &&
                      bus.read(dev.dynamic_address, back, 3) == I3cStatus::OK && back[2] == 9,
                      "assigned address does not reach the target");
}

static void run() {
  static DwI3cBus bus0(0), bus1(1), bus2(2);
  check_bus(bus0, 0x04a100001001ull, 0x06, 0x55);  // as configured on the QEMU command line
  check_bus(bus1, 0x04a100002002ull, 0x07, 0x66);

  I3cDevice devices[2];
  size_t none = 99;
  yesrtos_test::check(bus2.assign_dynamic_addresses(devices, 2, none) == I3cStatus::OK && none == 0,
                      "empty bus did not report zero targets");
  yesrtos_test::pass();
}

int main() {
  static Thread t(0, run);
  PreemptFIFOScheduler::add_thread(&t);
  PreemptFIFOScheduler::start();
  return 0;
}
