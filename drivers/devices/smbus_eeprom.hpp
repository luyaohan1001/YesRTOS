/**
 * @file smbus_eeprom.hpp
 * @brief Driver for small I2C/SMBus EEPROMs with an 8-bit word address (24C01/24C02 class, up to 256 bytes).
 */
#pragma once

#include "i2c_bus.hpp"

namespace YesRTOS {

class SmbusEeprom {
  public:
  /**
   * @param bus       Bus the EEPROM is on.
   * @param addr      7-bit address (0x50 .. 0x57).
   * @param size      Capacity in bytes (at most 256).
   * @param page_size Largest write the part accepts in one transaction without wrapping (8 or 16 on 24C0x parts).
   */
  SmbusEeprom(I2cBus& bus, uint8_t addr, uint32_t size = 256, uint32_t page_size = 16);

  /**
   * @brief Read `len` bytes starting at `offset` (sequential read).
   * @return BUS_ERROR without touching the bus if the range does not fit the EEPROM.
   */
  I2cStatus read(uint8_t offset, uint8_t* data, size_t len);

  /**
   * @brief Write `len` bytes starting at `offset`, split at page boundaries. After each page it polls the address until
   *        the EEPROM acknowledges again, i.e. finished its internal write cycle.
   * @return BUS_ERROR without touching the bus if the range does not fit the EEPROM.
   */
  I2cStatus write(uint8_t offset, const uint8_t* data, size_t len);

  private:
  static constexpr uint32_t MAX_PAGE_SIZE = 16;
  static constexpr uint32_t WRITE_CYCLE_POLLS = 1000;

  I2cStatus wait_write_cycle();

  I2cBus& bus;
  uint8_t address;
  uint32_t size;
  uint32_t page_size;
};

}  // namespace YesRTOS
