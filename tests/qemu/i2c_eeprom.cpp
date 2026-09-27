/**
 * @file i2c_eeprom.cpp
 * @brief AspeedI2cBus + SmbusEeprom against QEMU ast1030-evb's EEPROM (bus 0, 0x50): a write across page boundaries
 *        read back, a single byte read, NACK from an absent address with the bus still usable afterwards, and
 *        out-of-range requests rejected without a transfer.
 */
#include "aspeed_i2c.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "smbus_eeprom.hpp"
#include "test_support.hpp"

using namespace YesRTOS;

static void run() {
  static AspeedI2cBus bus0(0);
  static SmbusEeprom eeprom(bus0, 0x50);

  uint8_t pattern[40];
  for (uint32_t i = 0; i < sizeof(pattern); i++) pattern[i] = static_cast<uint8_t>(0x30 + 7 * i);
  yesrtos_test::check(eeprom.write(0x1c, pattern, sizeof(pattern)) == I2cStatus::OK, "write failed");  // spans 3 page boundaries

  uint8_t readback[40] = {0};
  yesrtos_test::check(eeprom.read(0x1c, readback, sizeof(readback)) == I2cStatus::OK, "read failed");
  for (uint32_t i = 0; i < sizeof(pattern); i++) {
    yesrtos_test::check(readback[i] == pattern[i], "read back differs from what was written");
  }

  uint8_t one = 0;
  yesrtos_test::check(eeprom.read(0x1c + 17, &one, 1) == I2cStatus::OK && one == pattern[17], "single byte read wrong");

  uint8_t probe = 0;
  yesrtos_test::check(bus0.read(0x33, &probe, 1) == I2cStatus::NACK, "absent device did not NACK");
  yesrtos_test::check(bus0.write(0x33, nullptr, 0) == I2cStatus::NACK, "absent device did not NACK an address probe");
  yesrtos_test::check(eeprom.read(0x1c, &one, 1) == I2cStatus::OK && one == pattern[0], "bus unusable after a NACK");

  yesrtos_test::check(eeprom.read(250, readback, 10) == I2cStatus::BUS_ERROR, "read past the end accepted");
  yesrtos_test::check(eeprom.write(250, pattern, 10) == I2cStatus::BUS_ERROR, "write past the end accepted");
  yesrtos_test::pass();
}

int main() {
  static Thread t(0, run);
  PreemptFIFOScheduler::add_thread(&t);
  PreemptFIFOScheduler::start();
  return 0;
}
