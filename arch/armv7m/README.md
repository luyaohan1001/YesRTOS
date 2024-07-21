# YesRTOS Machine Layer for ARMv7-M

This directory contains the machine layer for ARMv7-M.

The CMSIS / Core / Drivers and linker scripts are provided by STMicroelectronics through CubeMX code generator.

YesRTOS scheduling layer interacts with ARM CMSIS (Common Microcontroller Software Interface Standard) to complete the important kernel functionalities such as critical section protection and context switching.

## ARM Cortex M7
* In-order super-scalar processor - instruction level parallelism on dual issued instruction
* Dynamic branch prediction
* Thumb: compressed 16bit instruction set
* Banked stack pointer
* Harvard instruction I-cache & D-cache
* Memory protection unit (MPU)
* Automatic processor state saving and restoration on entry for exception handling
* NVIC: vectored interrupt handling.
* Breakpoint style debugging, instruction trace, etc.

## References

#### CMSIS SVD files for STM32 (Required for debug)
  https://github.com/modm-io/cmsis-svd-stm32

#### CMSIS for Cortex-M
  https://www.arm.com/technologies/cmsis

#### ARMv7-M Architecture Reference Manual
  https://developer.arm.com/documentation/ddi0403/d/Application-Level-Architecture

#### The-Cortex-M7-Instruction-Set
  https://developer.arm.com/documentation/dui0646/c/The-Cortex-M7-Instruction-Set

#### Calling assembly functions from C and C++
  https://developer.arm.com/documentation/100748/0622/Using-Assembly-and-Intrinsics-in-C-or-C---Code/Calling-assembly-functions-from-C-and-C--

#### Arm Cortex-M7 Devices Generic User Guide
  https://developer.arm.com/documentation/dui0646/c/The-Cortex-M7-Processor

#### Context switch requires knowledge of caller-saved registers and callee-saved registers.
  https://stackoverflow.com/questions/261419/what-registers-to-save-in-the-arm-c-calling-convention

## Notes

#### When migrating code base from C to C++
  1. When initilizating in C, initializing to a struct type, for example:
      someStruct_t m = {0} initializes all elements in the structs to 0.

  while in C++, this incurs compiler warnings.

  Example:
  [ 40%] Building C object src/armv7m/CMakeFiles/RT/Users/lhan/Projects/YesRTOS/src/armv7m/baremetal.cpp:129:44: warning: missing initializer for member 'RCC_ClkInitTypeDef::SYSCLKSource' [-Wmissing-field-initializers]
  129 |   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  2. When writing ARM cortex M7 assembly, the following directive is needed:

    .cpu cortex-m7

    to avoid following error:

    Error: lo register required -- `ldmia r0!,{r4-r11,r14}'

  3. Use of "extern" keyword in C++ and GNU ARM assembly.
    In both language, extern gives compiler hints that this symbol belongs to another library (another object file after object translation)

    If a symbol is defined as object type in assembly, for example

    .type  SVC_Handler, %function
    SVC_Handler:
      ........

    We can call this function in C++ by declaring

    extern "C" {
      extern void SVC_Handler()
    }

    If an argument needs to be passed, then it should be extern'd with arguments in its parameter list:

    extern "C" {
      extern void SVC_Handler(void *a)
    }

    then the address of a will be loaded into $r0 according to to ARM Procedure Call Standard.

  4. Why "extern C" key word is necessary, regarding C++ name mangling
      https://web.mit.edu/tibbetts/Public/inside-c/www/mangling.html

