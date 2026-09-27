#include "dw_i3c.hpp"

using namespace YesRTOS;

namespace {

constexpr uintptr_t I3C_BASE = 0x7E7A2000u;  // AST1030: controller n at I3C_BASE + 0x1000 * n
constexpr uintptr_t CONTROLLER_STRIDE = 0x1000u;
constexpr uint32_t IRQ_BASE = 102;            // controller n raises IRQ 102 + n (measured on QEMU ast1030-evb)

// Registers (byte offsets / 4).
constexpr uint32_t DEVICE_CTRL = 0x00 / 4;
constexpr uint32_t DEVICE_ADDR = 0x04 / 4;
constexpr uint32_t COMMAND_QUEUE_PORT = 0x0c / 4;
constexpr uint32_t RESPONSE_QUEUE_PORT = 0x10 / 4;
constexpr uint32_t RX_TX_DATA_PORT = 0x14 / 4;
constexpr uint32_t QUEUE_THLD_CTRL = 0x1c / 4;
constexpr uint32_t RESET_CTRL = 0x34 / 4;
constexpr uint32_t INTR_STATUS = 0x3c / 4;
constexpr uint32_t INTR_STATUS_EN = 0x40 / 4;
constexpr uint32_t INTR_SIGNAL_EN = 0x44 / 4;
constexpr uint32_t QUEUE_STATUS_LEVEL = 0x4c / 4;
constexpr uint32_t DEV_ADDR_TABLE = 0x280 / 4;  // 1 word per device

constexpr uint32_t DEVICE_CTRL_ENABLE = 1u << 31;
constexpr uint32_t DEVICE_CTRL_RESUME = 1u << 30;
constexpr uint32_t DEVICE_ADDR_DYNAMIC_VALID = 1u << 31;

constexpr uint32_t RESET_CTRL_QUEUES = (1u << 1) | (1u << 2) | (1u << 3) | (1u << 4);  // command, response, TX, RX
constexpr uint32_t RESET_CTRL_RX = 1u << 4;

constexpr uint32_t INTR_RESP_READY = 1u << 4;
constexpr uint32_t INTR_TRANSFER_ERR = 1u << 9;

// Command queue words.
constexpr uint32_t ATTR_TRANSFER_CMD = 0x0;
constexpr uint32_t ATTR_TRANSFER_ARG = 0x1;
constexpr uint32_t ATTR_ADDR_ASSIGN = 0x3;
constexpr uint32_t CMD_CP = 1u << 15;    // CMD field holds a CCC
constexpr uint32_t CMD_ROC = 1u << 26;   // response on completion
constexpr uint32_t CMD_READ = 1u << 28;
constexpr uint32_t CMD_TOC = 1u << 30;   // STOP after this command (else repeated START)

constexpr uint32_t cmd_tid(uint32_t tid) { return (tid & 0xf) << 3; }
constexpr uint32_t cmd_ccc(uint32_t ccc) { return (ccc & 0xff) << 7; }
constexpr uint32_t cmd_dev_index(uint32_t i) { return (i & 0x1f) << 16; }
constexpr uint32_t cmd_dev_count(uint32_t n) { return (n & 0x1f) << 21; }
constexpr uint32_t arg_data_length(uint32_t n) { return ATTR_TRANSFER_ARG | (n << 16); }

// Response queue word.
constexpr uint32_t resp_error(uint32_t r) { return r >> 28; }
constexpr uint32_t resp_tid(uint32_t r) { return (r >> 24) & 0xf; }
constexpr uint32_t resp_length(uint32_t r) { return r & 0xffff; }
constexpr uint32_t RESP_ERR_BROADCAST_NACK = 4;
constexpr uint32_t RESP_ERR_DAA_NACK = 5;
constexpr uint32_t RESP_ERR_NACK = 9;

// Dynamic addresses handed out by dynamic address assignment: slot n gets 0x10 + n (none is reserved by I3C).
constexpr uint8_t DAA_FIRST_ADDRESS = 0x10;

// Dynamic address with the odd parity bit I3C appends to it, as stored in the device address table.
uint32_t with_parity(uint8_t addr) {
  uint32_t ones = 0;
  for (uint32_t bits = addr; bits; bits >>= 1) ones += bits & 1u;
  return addr | ((ones & 1u) ? 0u : 0x80u);
}

#define NVIC_ISER(n) (*((volatile uint32_t*)(0xe000e100u + 4u * (n))))

DwI3cBus* instances[DwI3cBus::NUM_CONTROLLERS];

void dispatch(uint32_t controller) {
  if (instances[controller]) instances[controller]->handle_interrupt();
}

}  // namespace

DwI3cBus::DwI3cBus(uint32_t controller, uint8_t own_address) : responses_ready(0, 1), tid(0) {
  this->regs = reinterpret_cast<volatile uint32_t*>(I3C_BASE + CONTROLLER_STRIDE * controller);
  instances[controller] = this;
  for (uint32_t i = 0; i < MAX_DEVICES; i++) this->dat_used[i] = false;

  this->regs[RESET_CTRL] = RESET_CTRL_QUEUES;
  this->regs[INTR_STATUS_EN] = INTR_RESP_READY | INTR_TRANSFER_ERR;
  this->regs[INTR_SIGNAL_EN] = 0;
  this->regs[INTR_STATUS] = 0xffffffffu;
  this->regs[DEVICE_ADDR] = DEVICE_ADDR_DYNAMIC_VALID | (static_cast<uint32_t>(own_address) << 16);
  this->regs[DEVICE_CTRL] = DEVICE_CTRL_ENABLE;

  uint32_t irq = IRQ_BASE + controller;
  NVIC_ISER(irq / 32) = 1u << (irq % 32);
}

void DwI3cBus::handle_interrupt() {
  // RESP_READY stays set while responses are queued: mask the interrupt until the thread has taken them.
  this->regs[INTR_SIGNAL_EN] = 0;
  this->responses_ready.release();
}

void DwI3cBus::recover() {
  this->regs[RESET_CTRL] = RESET_CTRL_QUEUES;
  this->regs[INTR_STATUS] = INTR_TRANSFER_ERR;
  this->regs[DEVICE_CTRL] = this->regs[DEVICE_CTRL] | DEVICE_CTRL_ENABLE | DEVICE_CTRL_RESUME;
}

I3cStatus DwI3cBus::execute(Command* cmds, size_t count) {
  // Interrupt once every command has answered (threshold = count - 1), or on a transfer error.
  this->regs[QUEUE_THLD_CTRL] = static_cast<uint32_t>(count - 1) << 8;
  this->regs[INTR_SIGNAL_EN] = INTR_RESP_READY | INTR_TRANSFER_ERR;

  uint8_t tids[2];
  for (size_t c = 0; c < count; c++) {
    const Command& cmd = cmds[c];
    for (size_t i = 0; i < cmd.tx_len; i += 4) {
      uint32_t word = 0;
      for (size_t b = 0; b < 4 && i + b < cmd.tx_len; b++) word |= static_cast<uint32_t>(cmd.tx[i + b]) << (8 * b);
      this->regs[RX_TX_DATA_PORT] = word;
    }
    // TIDs 1..7: QEMU's model returns only 3 TID bits in the response (the command has 4), so stay below 8.
    this->tid = static_cast<uint8_t>(this->tid % 7 + 1);
    tids[c] = this->tid;
    this->regs[COMMAND_QUEUE_PORT] = cmd.argument;
    this->regs[COMMAND_QUEUE_PORT] = cmd.command | cmd_tid(tids[c]) | CMD_ROC;
  }

  this->responses_ready.acquire();  // released by handle_interrupt()

  I3cStatus status = I3cStatus::OK;
  size_t answered = (this->regs[QUEUE_STATUS_LEVEL] >> 8) & 0xff;
  for (size_t c = 0; c < count; c++) {
    if (c >= answered) {  // the controller halted before this command
      if (status == I3cStatus::OK) status = I3cStatus::ERROR;
      break;
    }
    uint32_t response = this->regs[RESPONSE_QUEUE_PORT];
    cmds[c].response = response;
    uint32_t error = resp_error(response);
    if (error == RESP_ERR_NACK || error == RESP_ERR_BROADCAST_NACK || error == RESP_ERR_DAA_NACK) {
      if (status == I3cStatus::OK) status = I3cStatus::NACK;
    } else if (error != 0 || resp_tid(response) != tids[c]) {
      status = I3cStatus::ERROR;
    }
    if (status == I3cStatus::OK && cmds[c].rx_len > 0) {
      size_t length = resp_length(response);
      for (size_t i = 0; i < length; i += 4) {
        uint32_t word = this->regs[RX_TX_DATA_PORT];
        for (size_t b = 0; b < 4 && i + b < length && i + b < cmds[c].rx_len; b++) {
          cmds[c].rx[i + b] = static_cast<uint8_t>(word >> (8 * b));
        }
      }
      if (length < cmds[c].rx_len) status = I3cStatus::ERROR;  // target ended the read early
    }
  }

  if (status != I3cStatus::OK) this->recover();
  return status;
}

int DwI3cBus::dat_slot(uint8_t addr) {
  int free_slot = -1;
  for (uint32_t i = 0; i < MAX_DEVICES; i++) {
    if (this->dat_used[i] && this->dat_address[i] == addr) return static_cast<int>(i);
    if (!this->dat_used[i] && free_slot < 0) free_slot = static_cast<int>(i);
  }
  if (free_slot >= 0) {
    this->dat_used[free_slot] = true;
    this->dat_address[free_slot] = addr;
    this->regs[DEV_ADDR_TABLE + free_slot] = with_parity(addr) << 16;
  }
  return free_slot;
}

I3cStatus DwI3cBus::do_daa(I3cDevice* devices, size_t max_devices, size_t& found) {
  found = 0;
  size_t slots = max_devices < MAX_DEVICES ? max_devices : MAX_DEVICES;
  if (slots == 0) return I3cStatus::ERROR;

  // RSTDAA just cleared every dynamic address: offer DAA_FIRST_ADDRESS + n in slots 0 .. slots - 1.
  for (uint32_t i = 0; i < MAX_DEVICES; i++) this->dat_used[i] = false;
  for (uint32_t i = 0; i < slots; i++) {
    this->regs[DEV_ADDR_TABLE + i] = with_parity(static_cast<uint8_t>(DAA_FIRST_ADDRESS + i)) << 16;
  }

  Command cmd = {0, ATTR_ADDR_ASSIGN | cmd_ccc(I3cCcc::ENTDAA) | cmd_dev_index(0) | cmd_dev_count(slots) | CMD_TOC,
                 nullptr, 0, nullptr, 0, 0};
  I3cStatus status = this->execute(&cmd, 1);

  // Some controllers (QEMU's model) also queue each target's PID/BCR/DCR as RX data: drop it.
  this->regs[RESET_CTRL] = RESET_CTRL_RX;

  // The response length is the number of offered addresses left over. Once no target is left the controller ends with
  // a broadcast / DAA NACK, which is how assignment normally finishes when fewer targets than slots exist.
  if (status == I3cStatus::ERROR) return status;
  size_t assigned = slots - resp_length(cmd.response);

  // Identify each target with GETPID / GETBCR / GETDCR, like the Linux I3C core does after assignment. This does not
  // depend on how the controller fills its device characteristic table (QEMU's model fills it inconsistently).
  for (size_t i = 0; i < assigned; i++) {
    uint8_t addr = static_cast<uint8_t>(DAA_FIRST_ADDRESS + i);
    this->dat_used[i] = true;
    this->dat_address[i] = addr;

    I3cDevice& dev = devices[i];
    dev.dynamic_address = addr;
    uint8_t pid[6];
    I3cStatus info = this->do_ccc(I3cCcc::GETPID, true, addr, nullptr, 0, pid, sizeof(pid));
    if (info == I3cStatus::OK) info = this->do_ccc(I3cCcc::GETBCR, true, addr, nullptr, 0, &dev.bcr, 1);
    if (info == I3cStatus::OK) info = this->do_ccc(I3cCcc::GETDCR, true, addr, nullptr, 0, &dev.dcr, 1);
    if (info != I3cStatus::OK) return info;
    dev.pid = decode_pid(pid);
  }
  found = assigned;
  return I3cStatus::OK;
}

I3cStatus DwI3cBus::do_private(uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata, size_t rlen) {
  if (wlen == 0 && rlen == 0) return I3cStatus::ERROR;
  int slot = this->dat_slot(addr);
  if (slot < 0) return I3cStatus::ERROR;

  Command cmds[2];
  size_t count = 0;
  if (wlen > 0) {
    cmds[count++] = {arg_data_length(wlen), ATTR_TRANSFER_CMD | cmd_dev_index(slot) | (rlen ? 0u : CMD_TOC), wdata, wlen,
                     nullptr, 0, 0};
  }
  if (rlen > 0) {
    cmds[count++] = {arg_data_length(rlen), ATTR_TRANSFER_CMD | cmd_dev_index(slot) | CMD_READ | CMD_TOC, nullptr, 0,
                     rdata, rlen, 0};
  }
  return this->execute(cmds, count);
}

I3cStatus DwI3cBus::do_ccc(uint8_t ccc, bool direct, uint8_t addr, const uint8_t* wdata, size_t wlen, uint8_t* rdata,
                           size_t rlen) {
  uint32_t index = 0;
  if (direct) {
    int slot = this->dat_slot(addr);
    if (slot < 0) return I3cStatus::ERROR;
    index = static_cast<uint32_t>(slot);
  }
  size_t length = rlen ? rlen : wlen;
  Command cmd = {arg_data_length(length),
                 ATTR_TRANSFER_CMD | CMD_CP | cmd_ccc(ccc) | cmd_dev_index(index) | (rlen ? CMD_READ : 0u) | CMD_TOC,
                 wdata, wlen, rdata, rlen, 0};
  return this->execute(&cmd, 1);
}

// Controller n interrupts on IRQ 102 + n.
#define DW_I3C_IRQ_HANDLER(irq) \
  extern "C" void IRQ##irq##_Handler(void) { dispatch(irq - IRQ_BASE); }
DW_I3C_IRQ_HANDLER(102)
DW_I3C_IRQ_HANDLER(103)
DW_I3C_IRQ_HANDLER(104)
DW_I3C_IRQ_HANDLER(105)
DW_I3C_IRQ_HANDLER(106)
DW_I3C_IRQ_HANDLER(107)
