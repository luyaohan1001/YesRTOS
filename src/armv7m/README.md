# YesRTOS Machine Layer for ARMv7-M

This directory contains the machine (dependent) layer for ARMv7-M.

The CMSIS / Core / Drivers and linker scripts are provided by STMicroelectronics through CubeMX code generator.

YesRTOS scheduling layer interacts with ARM CMSIS (Common Microcontroller Software Interface Standard) to complete the important kernel functionalities such as critical section protection and context switching.

## References

#### CMSIS SVD files for STM32 (Required for debug)
  https://github.com/modm-io/cmsis-svd-stm32

#### CMSIS for Cortex-M
  https://www.arm.com/technologies/cmsis

## Notes 

#### When migrating code base from C to C++
  1. When initilizating in C, initializing to a struct type, for example:
      someStruct_t m = {0} initializes all elements in the structs to 0.

  while in C++, this incurs compiler warnings.

  Example:
  [ 40%] Building C object src/armv7m/CMakeFiles/RT/Users/lhan/Projects/YesRTOS/src/armv7m/baremetal.cpp:129:44: warning: missing initializer for member 'RCC_ClkInitTypeDef::SYSCLKSource' [-Wmissing-field-initializers]
  129 |   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


