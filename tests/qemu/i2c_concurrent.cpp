/**
 * @file i2c_concurrent.cpp
 * @brief Two threads share I2C bus 0, each writing and reading back its own EEPROM region, while a third reads the
 *        TMP105 limits on bus 1. On every SysTick a higher priority thread wakes up (tick hook + semaphore) and reads
 *        its own EEPROM region too, preempting the others in the middle of their transactions. The bus mutex must keep
 *        every transaction whole: an interleaved START corrupts the interrupted transfer.
 * @note  QEMU completes each I2C command as soon as it is written, so without the tick preemption a transaction is
 *        never interrupted and the test would pass even without the bus mutex.
 */
#include "aspeed_i2c.hpp"
#include "atomic_section.hpp"
#include "preempt_fifo_scheduler.hpp"
#include "semaphore.hpp"
#include "smbus_eeprom.hpp"
#include "test_support.hpp"
#include "tmp105.hpp"

using namespace YesRTOS;

static const uint32_t ROUNDS = 1200;
static const uint32_t MIN_PREEMPTIONS = 30;
static AspeedI2cBus bus0(0), bus1(1);
static SmbusEeprom eeprom(bus0, 0x50);
static Tmp105 sensor(bus1, 0x4d);
static volatile uint32_t finished = 0;
static volatile uint32_t preemptions = 0;
static Semaphore tick(0, 1);

extern "C" void yesrtos_tick_hook(void) {
  tick.release();
}

static void finish() {
  atomic_section a;
  if (++finished == 3) {
    yesrtos_test::print("tick preemptions=");
    yesrtos_test::print_u32(preemptions);
    yesrtos_test::print("\n");
    yesrtos_test::check(preemptions >= MIN_PREEMPTIONS, "too few preemptions to exercise the bus mutex");
    yesrtos_test::pass();
  }
}

// Higher priority: runs on every tick, in the middle of whatever the others are doing on bus 0.
static void interrupter() {
  static const uint8_t mine[8] = {0xde, 0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67};
  yesrtos_test::check(eeprom.write(0xc0, mine, sizeof(mine)) == I2cStatus::OK, "interrupter write failed");
  while (1) {
    tick.acquire();
    uint8_t readback[8] = {0};
    yesrtos_test::check(eeprom.read(0xc0, readback, sizeof(readback)) == I2cStatus::OK, "interrupter read failed");
    for (uint32_t i = 0; i < sizeof(mine); i++) {
      yesrtos_test::check(readback[i] == mine[i], "interrupter's EEPROM data corrupted");
    }
    preemptions = preemptions + 1;
  }
}

static void eeprom_user(uint8_t base, uint8_t seed) {
  uint8_t data[24], readback[24];
  for (uint32_t round = 0; round < ROUNDS; round++) {
    for (uint32_t i = 0; i < sizeof(data); i++) data[i] = static_cast<uint8_t>(seed + round + 3 * i);
    yesrtos_test::check(eeprom.write(base, data, sizeof(data)) == I2cStatus::OK, "concurrent write failed");
    PreemptFIFOScheduler::yield();
    yesrtos_test::check(eeprom.read(base, readback, sizeof(readback)) == I2cStatus::OK, "concurrent read failed");
    for (uint32_t i = 0; i < sizeof(data); i++) {
      yesrtos_test::check(readback[i] == data[i], "EEPROM data corrupted by the other thread's transfers");
    }
    PreemptFIFOScheduler::yield();
  }
  finish();
}

static void user_a() { eeprom_user(0x40, 0x11); }
static void user_b() { eeprom_user(0x80, 0x77); }

static void sensor_user() {
  yesrtos_test::check(sensor.write_limits(-5000, 60000) == I2cStatus::OK, "limit write failed");
  for (uint32_t round = 0; round < ROUNDS; round++) {
    int32_t low = 0, high = 0;
    yesrtos_test::check(sensor.read_limits(low, high) == I2cStatus::OK, "limit read failed");
    yesrtos_test::check(low == -5000 && high == 60000, "TMP105 limits wrong during concurrent traffic");
    PreemptFIFOScheduler::yield();
  }
  finish();
}

int main() {
  static Thread a(0, user_a, 1), b(1, user_b, 1), c(2, sensor_user, 1), h(3, interrupter, 0);
  PreemptFIFOScheduler::add_thread(&h);
  PreemptFIFOScheduler::add_thread(&a);
  PreemptFIFOScheduler::add_thread(&b);
  PreemptFIFOScheduler::add_thread(&c);
  PreemptFIFOScheduler::start();
  return 0;
}
