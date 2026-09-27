#include "i3c_bus.hpp"

using namespace YesRTOS;

I3cStatus I3cBus::assign_dynamic_addresses(I3cDevice* devices, size_t max_devices, size_t& found) {
  this->lock.lock();
  found = 0;
  I3cStatus status = this->do_ccc(I3cCcc::RSTDAA, false, 0, nullptr, 0, nullptr, 0);
  // Nobody acknowledging RSTDAA just means the bus is empty; ENTDAA will tell.
  if (status == I3cStatus::OK || status == I3cStatus::NACK) {
    status = this->do_daa(devices, max_devices, found);
  }
  this->lock.unlock();
  return status;
}

I3cStatus I3cBus::write(uint8_t addr, const uint8_t* data, size_t len) {
  return this->write_read(addr, data, len, nullptr, 0);
}

I3cStatus I3cBus::read(uint8_t addr, uint8_t* data, size_t len) {
  return this->write_read(addr, nullptr, 0, data, len);
}

I3cStatus I3cBus::write_read(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen) {
  this->lock.lock();
  I3cStatus status = this->do_private(addr, wdata, wlen, rdata, rlen);
  this->lock.unlock();
  return status;
}

I3cStatus I3cBus::ccc_broadcast(uint8_t ccc, const uint8_t* data, size_t len) {
  this->lock.lock();
  I3cStatus status = this->do_ccc(ccc, false, 0, data, len, nullptr, 0);
  this->lock.unlock();
  return status;
}

I3cStatus I3cBus::ccc_get(uint8_t ccc, uint8_t addr, uint8_t* data, size_t len) {
  this->lock.lock();
  I3cStatus status = this->do_ccc(ccc, true, addr, nullptr, 0, data, len);
  this->lock.unlock();
  return status;
}

I3cStatus I3cBus::ccc_set(uint8_t ccc, uint8_t addr, const uint8_t* data, size_t len) {
  this->lock.lock();
  I3cStatus status = this->do_ccc(ccc, true, addr, data, len, nullptr, 0);
  this->lock.unlock();
  return status;
}

uint64_t I3cBus::decode_pid(const uint8_t bytes[6]) {
  uint64_t pid = 0;
  for (int i = 0; i < 6; i++) pid = (pid << 8) | bytes[i];
  return pid;
}
