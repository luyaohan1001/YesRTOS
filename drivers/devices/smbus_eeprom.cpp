#include "smbus_eeprom.hpp"

using namespace YesRTOS;

SmbusEeprom::SmbusEeprom(I2cBus& bus, uint8_t addr, uint32_t size, uint32_t page_size)
    : bus(bus), address(addr), size(size > 256 ? 256 : size), page_size(page_size > MAX_PAGE_SIZE ? MAX_PAGE_SIZE : page_size) {
}

I2cStatus SmbusEeprom::read(uint8_t offset, uint8_t* data, size_t len) {
  if (len == 0 || offset + len > this->size) return I2cStatus::BUS_ERROR;
  return this->bus.write_read(this->address, &offset, 1, data, len);
}

I2cStatus SmbusEeprom::write(uint8_t offset, const uint8_t* data, size_t len) {
  if (offset + len > this->size) return I2cStatus::BUS_ERROR;

  uint32_t position = offset;
  size_t done = 0;
  while (done < len) {
    // A page write must not cross a page boundary: the address counter wraps within the page.
    size_t chunk = this->page_size - position % this->page_size;
    if (chunk > len - done) chunk = len - done;

    uint8_t frame[1 + MAX_PAGE_SIZE];
    frame[0] = static_cast<uint8_t>(position);
    for (size_t i = 0; i < chunk; i++) frame[1 + i] = data[done + i];

    I2cStatus status = this->bus.write(this->address, frame, 1 + chunk);
    if (status != I2cStatus::OK) return status;
    status = this->wait_write_cycle();
    if (status != I2cStatus::OK) return status;

    position += chunk;
    done += chunk;
  }
  return I2cStatus::OK;
}

I2cStatus SmbusEeprom::wait_write_cycle() {
  // The part ignores its address (NACK) while it programs the page.
  for (uint32_t i = 0; i < WRITE_CYCLE_POLLS; i++) {
    I2cStatus status = this->bus.write(this->address, nullptr, 0);
    if (status != I2cStatus::NACK) return status;
  }
  return I2cStatus::NACK;
}
