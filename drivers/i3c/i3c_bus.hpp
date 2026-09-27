/**
 * @file i3c_bus.hpp
 * @brief Board independent MIPI I3C controller interface. Each board provides a controller driver deriving from it
 *        (e.g. DwI3cBus for the Synopsys DesignWare I3C controller).
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "mutex.hpp"

namespace YesRTOS {

enum class I3cStatus {
  OK,     // transfer completed
  NACK,   // no target acknowledged the address (or the broadcast, or dynamic address assignment)
  ERROR,  // CRC, parity or frame error, FIFO over/underflow, aborted transfer, or a request the controller cannot hold
};

/**
 * @brief A target found by dynamic address assignment.
 */
struct I3cDevice {
  uint8_t dynamic_address;
  uint64_t pid;  // 48-bit Provisioned ID
  uint8_t bcr;   // Bus Characteristics Register
  uint8_t dcr;   // Device Characteristics Register
};

/**
 * @brief Common Command Codes used by the drivers and tests. Broadcast CCCs are 0x00..0x7F, direct ones 0x80..0xFE.
 */
namespace I3cCcc {
constexpr uint8_t RSTDAA = 0x06;  // broadcast: forget all dynamic addresses
constexpr uint8_t ENTDAA = 0x07;  // broadcast: enter dynamic address assignment
constexpr uint8_t GETMWL = 0x8b;  // direct read: maximum write length (2 bytes, MSB first)
constexpr uint8_t GETMRL = 0x8c;  // direct read: maximum read length
constexpr uint8_t GETPID = 0x8d;  // direct read: Provisioned ID (6 bytes, MSB first)
constexpr uint8_t GETBCR = 0x8e;  // direct read: Bus Characteristics Register
constexpr uint8_t GETDCR = 0x8f;  // direct read: Device Characteristics Register
}  // namespace I3cCcc

class I3cBus {
  public:
  /**
   * @brief Reset all dynamic addresses (RSTDAA), then assign new ones to every target on the bus (ENTDAA).
   * @param devices     Filled with the targets found, in the order they won arbitration.
   * @param max_devices Room in `devices`.
   * @param found       Number of targets that got an address.
   */
  I3cStatus assign_dynamic_addresses(I3cDevice* devices, size_t max_devices, size_t& found);

  /**
   * @brief Private write / read to a target by dynamic address: START, address, data, STOP.
   */
  I3cStatus write(uint8_t addr, const uint8_t* data, size_t len);
  I3cStatus read(uint8_t addr, uint8_t* data, size_t len);

  /**
   * @brief Private write then read with a repeated START in between (no STOP), e.g. a register number then its value.
   */
  I3cStatus write_read(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen);

  /**
   * @brief Broadcast CCC with optional data, e.g. RSTDAA.
   */
  I3cStatus ccc_broadcast(uint8_t ccc, const uint8_t* data, size_t len);

  /**
   * @brief Direct CCC to one target: read (GETxxx) or write (SETxxx).
   */
  I3cStatus ccc_get(uint8_t ccc, uint8_t addr, uint8_t* data, size_t len);
  I3cStatus ccc_set(uint8_t ccc, uint8_t addr, const uint8_t* data, size_t len);

  /**
   * @brief Decode a GETPID response: 6 bytes, most significant byte first (MIPI I3C).
   */
  static uint64_t decode_pid(const uint8_t bytes[6]);

  protected:
  // Never deleted through an I3cBus*: protected, non-virtual destructor (see I2cBus for why).
  ~I3cBus() = default;

  // Controller specific parts, called with the bus mutex held, from a thread.
  virtual I3cStatus do_daa(I3cDevice* devices, size_t max_devices, size_t& found) = 0;
  virtual I3cStatus do_private(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen) = 0;
  virtual I3cStatus do_ccc(uint8_t ccc, bool direct, uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata,
                           size_t rlen) = 0;

  private:
  Mutex lock;
};

}  // namespace YesRTOS
