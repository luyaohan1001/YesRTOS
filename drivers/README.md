# YesRTOS drivers

```
drivers/
  i2c/i2c_bus.hpp          I2cBus: board independent I2C master interface
  i2c/aspeed_i2c.hpp       AspeedI2cBus: AST1030 I2C controller (QEMU ast1030-evb), interrupt driven
  devices/tmp105.hpp       Tmp105: TI TMP105 / LM75-class temperature sensor
  devices/smbus_eeprom.hpp SmbusEeprom: 24C01/24C02-class EEPROM with an 8-bit word address
```

Device drivers only use `I2cBus`, so they work unchanged on any board that provides a controller driver. The build
adds the controller driver of the selected `BOARD` (only `ast1030` has one so far).

## I2cBus

```cpp
I2cStatus write(uint8_t addr, const uint8_t* data, size_t len);        // START, addr+W, data, STOP
I2cStatus read(uint8_t addr, uint8_t* data, size_t len);               // START, addr+R, data, STOP
I2cStatus write_read(uint8_t addr, const uint8_t* wdata, size_t wlen,  // e.g. register number, then its value,
                     uint8_t* rdata, size_t rlen);                     // with a repeated START in between
```

Addresses are 7-bit. Every call is one complete transaction and returns `OK`, `NACK` (address or byte not
acknowledged) or `BUS_ERROR`. A per-bus `Mutex` serialises the threads sharing a bus, so a transaction is never
interleaved with another one, even when a higher priority thread preempts in the middle of it.

## AspeedI2cBus

```cpp
static AspeedI2cBus bus0(0);          // bus number 0..13
static SmbusEeprom eeprom(bus0, 0x50);
static AspeedI2cBus bus1(1);
static Tmp105 sensor(bus1, 0x4d);
```

Byte mode of the controller: each START, byte and STOP is one command. The calling thread issues the command and
blocks on a semaphore; the bus interrupt (IRQ 110 + bus number) latches the status and releases it, so other threads
run while a transfer is in progress. Use it from threads, after `PreemptFIFOScheduler::start()`.

QEMU ast1030-evb attaches an SMBus EEPROM at 0x50 on bus 0 and a TMP105 at 0x4D on bus 1 (it reports 0 C). The tests
`tests/qemu/i2c_*` run against them: `ctest --preset qemu-ast1030`.

## Limitations

- No transfer timeout: if the controller never raises its interrupt, the calling thread blocks forever (KI-015).
- STM32F767 has no I2C controller driver yet; QEMU does not emulate the STM32 I2C peripheral.
