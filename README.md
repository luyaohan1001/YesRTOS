# YesRTOS - A Real-Time Operating System

![alt text](/docs/images/github_profile.jpg)

[AI art generated with Magic Studio](https://magicstudio.com/ai-art-generator/)

## Goal of the Project
* Lightweight, efficient, and low coupling real time operating system.

## Language
* Modern C++, C11, and ARM assembly.

## Build System
* CMake

## Tool Chain
* VSCode + GNU GCC Cross Compiler, Analysis Tools, etc.
* OpenOCD
* GDB

## Design Phase of YesRTOS
#### Phase 0
* Round-Robin Scheduler
* Cooperative Thread (no preemption)
* Context Switch

#### Phase 1
* Timeslice
* Memory Management
* Preemptive Thread
* Priority Scheduler

#### Phase 2
* Profiling Interface
* Software Architecture Layout

#### Phase 3
* Spinlock
* Mutex
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