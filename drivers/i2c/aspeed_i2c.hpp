/**
 * @file aspeed_i2c.hpp
 * @brief I2C master driver for the Aspeed I2C controller of the AST1030 (QEMU ast1030-evb), interrupt driven.
 * @note  Uses the controller's byte mode ("old" register mode, the reset default): one command per START, byte or
 *        STOP. The calling thread issues a command and blocks on a semaphore; the bus interrupt (IRQ 110 + bus) records
 *        the status and releases it. Must be used from threads, after PreemptFIFOScheduler::start().
 */
#pragma once

#include "i2c_bus.hpp"
#include "semaphore.hpp"

namespace YesRTOS {

class AspeedI2cBus final : public I2cBus {
  public:
  static constexpr uint32_t NUM_BUSES = 14;

  /**
   * @param bus Bus number, 0 .. NUM_BUSES - 1. Enables the bus as master and its interrupt.
   */
  explicit AspeedI2cBus(uint32_t bus);

  /**
   * @brief Interrupt handler body for this bus: latches and clears the status, wakes the waiting thread.
   */
  void handle_interrupt();

  protected:
  I2cStatus transfer(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen) override;

  private:
  // Issue one command (with `byte` in the transmit buffer for START/TX) and wait for its interrupt; returns the status.
  uint32_t command(uint32_t cmd, uint8_t byte = 0);

  // Send STOP; returns `result`, or BUS_ERROR if the STOP itself failed.
  I2cStatus finish(I2cStatus result);

  volatile uint32_t* regs;
  uint32_t bus_number;
  Semaphore step_done;
  volatile uint32_t step_status;
};

}  // namespace YesRTOS
