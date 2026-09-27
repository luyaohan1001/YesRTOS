#include "i2c_bus.hpp"

using namespace YesRTOS;

I2cStatus I2cBus::write(uint8_t addr, const uint8_t* data, size_t len) {
  return this->write_read(addr, data, len, nullptr, 0);
}

I2cStatus I2cBus::read(uint8_t addr, uint8_t* data, size_t len) {
  return this->write_read(addr, nullptr, 0, data, len);
}

I2cStatus I2cBus::write_read(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen) {
  this->lock.lock();
  I2cStatus status = this->transfer(addr, wdata, wlen, rdata, rlen);
  this->lock.unlock();
  return status;
}
