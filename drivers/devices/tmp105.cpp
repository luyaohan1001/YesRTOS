#include "tmp105.hpp"

using namespace YesRTOS;

namespace {

// Temperature registers are 16-bit two's complement, MSB first, in 1/256 C: the integer degrees in the first byte and
// the fraction in the upper bits of the second (12 significant bits, 0.0625 C per step).
int32_t to_millicelsius(uint8_t msb, uint8_t lsb) {
  int16_t raw = static_cast<int16_t>(static_cast<uint16_t>(msb << 8) | lsb);
  return static_cast<int32_t>(raw) * 1000 / 256;
}

uint16_t from_millicelsius(int32_t millicelsius) {
  int32_t raw = millicelsius * 256 / 1000;
  return static_cast<uint16_t>(raw & 0xfff0);  // only the 12 significant bits are kept by the sensor
}

}  // namespace

Tmp105::Tmp105(I2cBus& bus, uint8_t addr) : bus(bus), address(addr) {
}

I2cStatus Tmp105::read_temperature(int32_t& millicelsius) {
  return this->read_register(REG_TEMPERATURE, millicelsius);
}

I2cStatus Tmp105::read_limits(int32_t& low_millicelsius, int32_t& high_millicelsius) {
  I2cStatus status = this->read_register(REG_T_LOW, low_millicelsius);
  if (status != I2cStatus::OK) return status;
  return this->read_register(REG_T_HIGH, high_millicelsius);
}

I2cStatus Tmp105::write_limits(int32_t low_millicelsius, int32_t high_millicelsius) {
  I2cStatus status = this->write_register(REG_T_LOW, low_millicelsius);
  if (status != I2cStatus::OK) return status;
  return this->write_register(REG_T_HIGH, high_millicelsius);
}

I2cStatus Tmp105::read_register(uint8_t reg, int32_t& millicelsius) {
  uint8_t value[2];
  I2cStatus status = this->bus.write_read(this->address, &reg, 1, value, 2);
  if (status == I2cStatus::OK) {
    millicelsius = to_millicelsius(value[0], value[1]);
  }
  return status;
}

I2cStatus Tmp105::write_register(uint8_t reg, int32_t millicelsius) {
  uint16_t raw = from_millicelsius(millicelsius);
  uint8_t frame[3] = {reg, static_cast<uint8_t>(raw >> 8), static_cast<uint8_t>(raw)};
  return this->bus.write(this->address, frame, sizeof(frame));
}
