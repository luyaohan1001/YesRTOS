![CI WORKFLOW STATUS BADGE](https://github.com/luyaohan1001/YesRTOS/actions/workflows/cmake-single-platform.yml/badge.svg?branch=main)

# YesRTOS - A Real-Time Operating System

![alt text](/docs/diagrams/github_profile.jpg)

[AI art generated with Magic Studio](https://magicstudio.com/ai-art-generator/)

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

  Follow these steps to compile this project.
  ```bash
  1. mkdir build
  2. cd build
  3. cmake ..
  4. make -j8
  ```

  To flash to platform using OpenOCD, use the following command:
  ```bash
  make flash
  ```

---

