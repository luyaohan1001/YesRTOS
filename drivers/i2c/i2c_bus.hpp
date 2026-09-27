/**
 * @file i2c_bus.hpp
 * @brief Board independent I2C master interface. Device drivers (drivers/devices) are written against it; each board
 *        provides a controller driver deriving from it (e.g. AspeedI2cBus).
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "mutex.hpp"

namespace YesRTOS {

enum class I2cStatus {
  OK,         // transfer completed
  NACK,       // the device did not acknowledge its address or a byte
  BUS_ERROR,  // arbitration lost, abnormal STOP/START or bus timeout
};

class I2cBus {
  public:
  /**
   * @brief Write `len` bytes to the 7-bit address `addr`: START, address + W, data, STOP. `len` 0 only checks that the
   *        device acknowledges its address.
   */
  I2cStatus write(uint8_t addr, const uint8_t* data, size_t len);

  /**
   * @brief Read `len` bytes (at least 1) from `addr`: START, address + R, data (NACK on the last byte), STOP.
   */
  I2cStatus read(uint8_t addr, uint8_t* data, size_t len);

  /**
   * @brief Write then read in one transaction with a repeated START, e.g. a register number then its value.
   */
  I2cStatus write_read(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen);

  protected:
  // Buses are never deleted through an I2cBus*: a protected, non-virtual destructor forbids that at compile time and
  // keeps the compiler from emitting a deleting destructor, which would pull operator delete, free() and newlib's
  // _sbrk into this bare-metal image.
  ~I2cBus() = default;

  /**
   * @brief One transaction: START, write phase (if wlen, or if no read phase), repeated START and read phase (if rlen),
   *        STOP. Always ends with STOP, also after a NACK.
   * @note  Called with the bus mutex held, from a thread.
   */
  virtual I2cStatus transfer(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen) = 0;

  private:
  // Serialises transactions of threads sharing the bus: a transaction is never interleaved with another one.
  Mutex lock;
};

}  // namespace YesRTOS
