/**
 * @file dw_i3c.hpp
 * @brief Driver for the Synopsys DesignWare I3C controller in controller (master) mode, as used by the Aspeed AST1030
 *        (QEMU ast1030-evb: 6 controllers at 0x7E7A2000 + 0x1000 * n, IRQ 102 + n). Interrupt driven.
 * @note  Commands go through the command queue with a response requested for each; the calling thread blocks on a
 *        semaphore until the response threshold interrupt (or a transfer error) arrives. Use from threads, after
 *        PreemptFIFOScheduler::start().
 */
#pragma once

#include "i3c_bus.hpp"
#include "semaphore.hpp"

namespace YesRTOS {

class DwI3cBus final : public I3cBus {
  public:
  static constexpr uint32_t NUM_CONTROLLERS = 6;
  static constexpr uint32_t MAX_DEVICES = 8;  // device address table depth on the AST1030

  /**
   * @param controller    Controller number, 0 .. NUM_CONTROLLERS - 1.
   * @param own_address   Dynamic address of the controller itself.
   */
  explicit DwI3cBus(uint32_t controller, uint8_t own_address = 0x08);

  /**
   * @brief Interrupt handler body: masks the (level triggered) interrupt and wakes the waiting thread.
   */
  void handle_interrupt();

  protected:
  I3cStatus do_daa(I3cDevice* devices, size_t max_devices, size_t& found) override;
  I3cStatus do_private(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen) override;
  I3cStatus do_ccc(uint8_t ccc, bool direct, uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata,
                   size_t rlen) override;

  private:
  struct Command {
    uint32_t argument;    // transfer argument (data length), 0 for address assignment
    uint32_t command;     // command word, without TID and ROC (added by execute())
    const uint8_t* tx;    // bytes for the TX FIFO
    size_t tx_len;
    uint8_t* rx;          // where the received bytes go
    size_t rx_len;
    uint32_t response;    // filled by execute()
  };

  // Submit the commands (at most 2), wait for their responses and collect received data.
  I3cStatus execute(Command* cmds, size_t count);

  // Recover from a transfer error: flush the queues and resume the halted controller.
  void recover();

  // Device address table slot holding `addr` as dynamic address, adding it if needed; -1 if the table is full.
  int dat_slot(uint8_t addr);

  volatile uint32_t* regs;
  Semaphore responses_ready;
  uint8_t tid;
  uint8_t dat_address[MAX_DEVICES];
  bool dat_used[MAX_DEVICES];
};

}  // namespace YesRTOS
