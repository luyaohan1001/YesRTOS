/**
 * @file i2c_tmp105.cpp
 * @brief AspeedI2cBus + Tmp105 against QEMU ast1030-evb's TMP105 (bus 1, 0x4D): temperature read (QEMU reports 0 C),
 *        alert limits written and read back (negative and fractional values exercise the conversion), and NACK at a
 *        wrong address.
 */
#include "aspeed_i2c.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "test_support.hpp"
#include "tmp105.hpp"

using namespace YesRTOS;

static void run() {
  static AspeedI2cBus bus1(1);
  static Tmp105 sensor(bus1, 0x4d);

  int32_t temperature = -1;
  yesrtos_test::check(sensor.read_temperature(temperature) == I2cStatus::OK, "temperature read failed");
  yesrtos_test::check(temperature == 0, "unexpected temperature (QEMU's TMP105 reports 0 C)");

  yesrtos_test::check(sensor.write_limits(-12500, 90250) == I2cStatus::OK, "limit write failed");
  int32_t low = 0, high = 0;
  yesrtos_test::check(sensor.read_limits(low, high) == I2cStatus::OK, "limit read failed");
  yesrtos_test::print("T_LOW=");
  yesrtos_test::print(low < 0 ? "-" : "");
  yesrtos_test::print_u32(static_cast<uint32_t>(low < 0 ? -low : low));
  yesrtos_test::print(" T_HIGH=");
  yesrtos_test::print_u32(static_cast<uint32_t>(high));
  yesrtos_test::print(" (milli C)\n");
  yesrtos_test::check(low == -12500 && high == 90250, "limits read back differ from what was written");

  static Tmp105 absent(bus1, 0x48);
  yesrtos_test::check(absent.read_temperature(temperature) == I2cStatus::NACK, "absent sensor did not NACK");
  yesrtos_test::pass();
}

int main() {
  static Thread t(0, run);
  PreemptFIFOScheduler::add_thread(&t);
  PreemptFIFOScheduler::start();
  return 0;
}
