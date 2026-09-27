/**
 * @file tmp105.hpp
 * @brief Driver for the TI TMP105 digital temperature sensor (and register compatible LM75/TMP75 parts).
 */
#pragma once

#include "i2c_bus.hpp"

namespace YesRTOS {

class Tmp105 {
  public:
  /**
   * @param bus  Bus the sensor is on.
   * @param addr 7-bit address, 0x48 .. 0x4F depending on its address pins.
   */
  Tmp105(I2cBus& bus, uint8_t addr);

  /**
   * @brief Current temperature in thousandths of a degree Celsius (0.0625 C steps at 12-bit resolution).
   */
  I2cStatus read_temperature(int32_t& millicelsius);

  /**
   * @brief Alert thresholds T_LOW / T_HIGH, same unit and format as the temperature (12-bit, 0.0625 C steps).
   */
  I2cStatus read_limits(int32_t& low_millicelsius, int32_t& high_millicelsius);
  I2cStatus write_limits(int32_t low_millicelsius, int32_t high_millicelsius);

  private:
  // Pointer register values.
  static constexpr uint8_t REG_TEMPERATURE = 0x00;
  static constexpr uint8_t REG_T_LOW = 0x02;
  static constexpr uint8_t REG_T_HIGH = 0x03;

  I2cStatus read_register(uint8_t reg, int32_t& millicelsius);
  I2cStatus write_register(uint8_t reg, int32_t millicelsius);

  I2cBus& bus;
  uint8_t address;
};

}  // namespace YesRTOS
