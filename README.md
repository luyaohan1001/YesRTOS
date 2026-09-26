![CI WORKFLOW STATUS BADGE](https://github.com/luyaohan1001/YesRTOS/actions/workflows/cmake-single-platform.yml/badge.svg?branch=main)

# YesRTOS - A Real-Time Operating System

![alt text](/docs/diagrams/YesRTOS_LOGO.png)

## Goal of the Project
* Lightweight real time operating system for ARM-Cortex M.

## Language
* C++17
* C11
* [GNU ARM assembly with C Expression Operands](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)

## Build System
* CMake, Make

## Environment / Tool Chain
* VSCode
* Cortex-Debug Extension
* OpenOCD
* GNU ARM Toolchain
* PlantUML
* Doxygen
* Cppcheck

## Tracing
* ARM ITM/SWO

## YesRTOS Architecture
* The kernel follows modular pattern with various *static* libraries:
  * LibStartupASM
  * LibBareMetal
  * LibYesRTOSKernel.

![alt text](/docs/diagrams/architecture.png)

## Design Phase of YesRTOS
#### Phase 0
* Round-Robin Scheduler
* Cooperative Thread (no preemption)
* Context Switch

#### Phase 1
* Timeslice
* Memory Management
* Mutex


#### Phase 2
* Priority Scheduler
* Preemptive Thread
* Profiling Interface
* Software Architecture Layout

#### Phase 3
* Spinlock
* Semaphore

#### Phase 4
* Priority Inheritance
* Priority Ceiling
* Message Queue
* Exception Handling

#### Phase 5
* Other advanced topics

## Intended Application in Future
* ⌚️ IoT Devices
* 🚇 Automotive Systems
* 👨‍🏭 Industrial Automation
* 🏥 Medical Devices
* 🎮 Consumer Electronics


## Phase 0 Demo

![alt text](/docs/demo/CooperativeMultitaskingDemo.gif)
CooperativeMultitaskingDemo.mp4

#### Compilation

  The build is driven by CMake presets (`CMakePresets.json`); each preset builds into `build/<preset>`.
  ```bash
  cmake --preset stm32f767          # or: qemu, qemu-stress
  cmake --build --preset stm32f767
  ```

  | Preset        | Target                                                  |
  |---------------|---------------------------------------------------------|
  | `stm32f767`   | STM32F767 hardware (Cortex-M7)                          |
  | `qemu`        | QEMU `netduinoplus2` (Cortex-M4F), no hardware needed   |
  | `qemu-stress` | Same as `qemu` with a 10 kHz timeslice to stress preemption |

  To flash to platform using OpenOCD, use the following command:
  ```bash
  cmake --build --preset stm32f767 --target flash
  ```

  To run on QEMU (exit with `Ctrl-A X`), or halt at reset waiting for gdb on `localhost:1234`:
  ```bash
  cmake --preset qemu
  cmake --build --preset qemu --target qemu
  cmake --build --preset qemu --target qemu-gdb
  ```

---

