#include "aspeed_i2c.hpp"

using namespace YesRTOS;

namespace {

// Controller at 0x7E7B0000: global registers, then one 0x80 byte register block per bus starting at +0x80.
constexpr uintptr_t I2C_BASE = 0x7E7B0000u;
constexpr uintptr_t BUS_STRIDE = 0x80u;
constexpr uint32_t IRQ_BASE = 110;  // bus n raises IRQ 110 + n (measured on QEMU ast1030-evb)

// Per bus registers (byte offsets / 4), old register mode.
constexpr uint32_t FUN_CTRL = 0x00 / 4;   // function control
constexpr uint32_t INTR_CTRL = 0x0c / 4;  // interrupt enable
constexpr uint32_t INTR_STS = 0x10 / 4;   // interrupt status, write 1 to clear. Bits only latch while enabled.
constexpr uint32_t CMD = 0x14 / 4;        // command
constexpr uint32_t BYTE_BUF = 0x20 / 4;   // [7:0] byte to transmit, [15:8] byte received

constexpr uint32_t FUN_CTRL_MASTER_EN = 1u << 0;

constexpr uint32_t INTR_TX_ACK = 1u << 0;
constexpr uint32_t INTR_TX_NAK = 1u << 1;
constexpr uint32_t INTR_RX_DONE = 1u << 2;
constexpr uint32_t INTR_ARBIT_LOSS = 1u << 3;
constexpr uint32_t INTR_NORMAL_STOP = 1u << 4;
constexpr uint32_t INTR_ABNORMAL = 1u << 5;
constexpr uint32_t INTR_SCL_TIMEOUT = 1u << 6;
constexpr uint32_t INTR_ERRORS = INTR_ARBIT_LOSS | INTR_ABNORMAL | INTR_SCL_TIMEOUT;
constexpr uint32_t INTR_ALL = INTR_TX_ACK | INTR_TX_NAK | INTR_RX_DONE | INTR_NORMAL_STOP | INTR_ERRORS;

constexpr uint32_t CMD_START = 1u << 0;
constexpr uint32_t CMD_TX = 1u << 1;
constexpr uint32_t CMD_RX = 1u << 3;
constexpr uint32_t CMD_RX_LAST = 1u << 4;  // NACK the received byte: last byte of a read
constexpr uint32_t CMD_STOP = 1u << 5;

#define NVIC_ISER(n) (*((volatile uint32_t*)(0xe000e100u + 4u * (n))))

AspeedI2cBus* instances[AspeedI2cBus::NUM_BUSES];

void dispatch(uint32_t bus) {
  if (instances[bus]) instances[bus]->handle_interrupt();
}

}  // namespace

AspeedI2cBus::AspeedI2cBus(uint32_t bus) : bus_number(bus), step_done(0, 1), step_status(0) {
  this->regs = reinterpret_cast<volatile uint32_t*>(I2C_BASE + BUS_STRIDE * (bus + 1));
  instances[bus] = this;

  this->regs[FUN_CTRL] = FUN_CTRL_MASTER_EN;
  this->regs[INTR_STS] = 0xffffffffu;
  this->regs[INTR_CTRL] = INTR_ALL;
  uint32_t irq = IRQ_BASE + bus;
  NVIC_ISER(irq / 32) = 1u << (irq % 32);
}

void AspeedI2cBus::handle_interrupt() {
  uint32_t status = this->regs[INTR_STS];
  this->regs[INTR_STS] = status;  // write 1 to clear
  this->step_status = status;
  this->step_done.release();
}

uint32_t AspeedI2cBus::command(uint32_t cmd, uint8_t byte) {
  if (cmd & (CMD_START | CMD_TX)) {
    this->regs[BYTE_BUF] = byte;
  }
  this->regs[CMD] = cmd;
  this->step_done.acquire();  // released by handle_interrupt()
  return this->step_status;
}

I2cStatus AspeedI2cBus::finish(I2cStatus result) {
  uint32_t status = this->command(CMD_STOP);
  if (!(status & INTR_NORMAL_STOP) || (status & INTR_ERRORS)) return I2cStatus::BUS_ERROR;
  return result;
}

/**
 * @brief Outcome of one step: OK when the step's success bit is set, NACK when the device did not acknowledge, and
 *        BUS_ERROR on an error bit or any other unexpected status.
 */
static I2cStatus classify(uint32_t status, uint32_t expected) {
  if (status & INTR_ERRORS) return I2cStatus::BUS_ERROR;
  if (status & INTR_TX_NAK) return I2cStatus::NACK;
  if (!(status & expected)) return I2cStatus::BUS_ERROR;
  return I2cStatus::OK;
}

I2cStatus AspeedI2cBus::transfer(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen) {
  I2cStatus result;

  if (wlen > 0 || rlen == 0) {
    result = classify(this->command(CMD_START | CMD_TX, static_cast<uint8_t>(addr << 1)), INTR_TX_ACK);
    for (size_t i = 0; i < wlen && result == I2cStatus::OK; i++) {
      result = classify(this->command(CMD_TX, wdata[i]), INTR_TX_ACK);
    }
    if (result != I2cStatus::OK) return this->finish(result);
  }

  if (rlen > 0) {
    // (Repeated) START with the read bit.
    result = classify(this->command(CMD_START | CMD_TX, static_cast<uint8_t>((addr << 1) | 1u)), INTR_TX_ACK);
    if (result != I2cStatus::OK) return this->finish(result);
    for (size_t i = 0; i < rlen; i++) {
      result = classify(this->command(i + 1 == rlen ? (CMD_RX | CMD_RX_LAST) : CMD_RX), INTR_RX_DONE);
      if (result != I2cStatus::OK) return this->finish(result);
      rdata[i] = static_cast<uint8_t>(this->regs[BYTE_BUF] >> 8);
    }
  }

  return this->finish(I2cStatus::OK);
}

// Bus n interrupts on IRQ 110 + n.
#define ASPEED_I2C_IRQ_HANDLER(irq) \
  extern "C" void IRQ##irq##_Handler(void) { dispatch(irq - IRQ_BASE); }
ASPEED_I2C_IRQ_HANDLER(110)
ASPEED_I2C_IRQ_HANDLER(111)
ASPEED_I2C_IRQ_HANDLER(112)
ASPEED_I2C_IRQ_HANDLER(113)
ASPEED_I2C_IRQ_HANDLER(114)
ASPEED_I2C_IRQ_HANDLER(115)
ASPEED_I2C_IRQ_HANDLER(116)
ASPEED_I2C_IRQ_HANDLER(117)
ASPEED_I2C_IRQ_HANDLER(118)
ASPEED_I2C_IRQ_HANDLER(119)
ASPEED_I2C_IRQ_HANDLER(120)
ASPEED_I2C_IRQ_HANDLER(121)
ASPEED_I2C_IRQ_HANDLER(122)
ASPEED_I2C_IRQ_HANDLER(123)
