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

## Case Studies

Real bugs found while building YesRTOS, kept as reminders of RTOS design rules. Each links to the hazard, a minimal
example, the fix and how it was found.

<details>
<summary><b>15 case studies</b> (race conditions, atomicity, memory ordering, interrupt priority, undefined behaviour, stack usage) - click to expand</summary>

| ID | Issue | Category | Design rule | Status |
|----|-------|----------|-------------|--------|
| [CS-001](docs/case_studies/CS-001-mutex-wakeup-race.md) | Mutex wake-up without ownership | Race condition | Hand a released lock directly to a waiter; test lock implementations by forcing contention, not by hoping for it. | Fixed `c4067fe` |
| [CS-002](docs/case_studies/CS-002-scheduling-through-stale-links.md) | Scheduling through a blocked thread's list links | Data integrity | A node moved to another list must not be traversed through its old links; act on the node you moved, not on the current thread. | Fixed `1970288` |
| [CS-003](docs/case_studies/CS-003-weak-split-cas.md) | Compare-and-swap split across functions | Atomicity | Keep an LDREX/STREX pair in a single asm block, and retry lost reservations so a failed CAS always means a different value. | Fixed `c1c34b6` |
| [CS-004](docs/case_studies/CS-004-non-nestable-critical-section.md) | Critical section re-enables interrupts when nested | Race condition | Restore the saved interrupt state instead of forcing it on; never block inside a critical section. | Fixed `a57be71` |
| [CS-005](docs/case_studies/CS-005-interrupt-disable-without-compiler-barrier.md) | Disabling interrupts is not a compiler barrier | Memory ordering | Every instruction that starts or ends a critical section must also be a compiler barrier. | Fixed `334c620` |
| [CS-006](docs/case_studies/CS-006-spinlock-on-single-core.md) | Spinlock across priorities on a single core | Deadlock | On a single core, block instead of spinning; spinlocks belong to multi-core code. | Documented |
| [CS-007](docs/case_studies/CS-007-unaligned-thread-stacks.md) | Thread stacks not 8-byte aligned | Undefined behaviour (ABI) | A thread's initial stack pointer must satisfy the ABI's alignment: align the stack storage, not just its size. | Fixed `bececfb` |
| [CS-008](docs/case_studies/CS-008-extended-asm-in-naked-handlers.md) | Extended asm in naked exception handlers | Undefined behaviour (compiler dependent) | Naked handlers contain only basic asm; call a normal C function for anything the compiler must generate. | Fixed `c1aef96` |
| [CS-009](docs/case_studies/CS-009-pendsv-at-highest-priority.md) | PendSV and SysTick at the highest priority | Interrupt priority | The context switch exception runs at the lowest priority, after every interrupt handler has finished. | Fixed `1099c2a` |
| [CS-010](docs/case_studies/CS-010-empty-ready-set.md) | No thread ready: count_trailing_zero(0) | Undefined behaviour | The ready set must never be empty: give the scheduler an idle thread. | Fixed `1ff6d5b` |
| [CS-011](docs/case_studies/CS-011-thread-routine-return.md) | Thread routine returning into 0xDEADBEEF | Undefined behaviour | Every thread needs a defined way to end: point its initial return address at an exit routine. | Fixed `34446ac` |
| [CS-012](docs/case_studies/CS-012-unchecked-add-thread.md) | add_thread() without validation or locking | Data integrity | Validate every index that comes from the API before using it, and treat public kernel calls as callable at any time. | Fixed `2f7410e` |
| [CS-013](docs/case_studies/CS-013-linkedlist-unconstructed-nodes.md) | Linked list nodes used without construction | Undefined behaviour | Memory from an allocator is not an object until it is constructed; an allocation failure is a return value, not an assert. | Fixed `e5b77ee` |
| [CS-014](docs/case_studies/CS-014-fpu-context-not-saved.md) | FPU context not saved on context switch | Data integrity | If threads may use the FPU, the FPU registers are part of the thread context. | **Open** |
| [CS-015](docs/case_studies/CS-015-thread-objects-on-main-stack.md) | Thread objects on the main stack, no stack reserve | Stack overflow | Size and reserve every stack explicitly, including the exception stack; do not put thread stacks on it. | **Open** |

</details>

## Known Issues

Open problems in the kernel, build and tests. Fixed issues are removed from this table; the ones worth remembering
become case studies above.

<details>
<summary><b>7 open issues</b> - click to expand</summary>

| ID | Issue | Area | Details | Status |
|----|-------|------|---------|--------|
| KI-001 | FPU registers (`s16`-`s31`, extended frame) are not saved on context switch, and the FPU is never enabled (no CPACR write) | Context switch | [CS-014](docs/case_studies/CS-014-fpu-context-not-saved.md) | Open |
| KI-002 | Demo thread stacks live on MSP; linker scripts reserve no MSP space (`_alloc_stack_size = 0x0`); no overflow detection | Memory layout | [CS-015](docs/case_studies/CS-015-thread-objects-on-main-stack.md) | Open |
| KI-003 | Mutex has no priority inheritance: a low priority owner can be starved by medium priority threads while a high priority thread waits (the `owner` comment promises it) | Mutex |  | Open |
| KI-004 | `RoundRobinScheduler` cannot run: PendSV/SVC only use `PreemptFIFOScheduler::p_active_thread`, and its `add_thread()` does not bound `TASK_QUEUE_DEPTH` | Scheduler |  | Open |
| KI-007 | Semaphore not implemented (`kernel/src/semaphore.cpp` is empty and not built) | Kernel API |  | Open |
| KI-008 | Host unit test `unit_tests/linkedlist_unit_tests` does not build: `thread.hpp` requires an architecture | Tests |  | Open |
| KI-009 | Demo passes id 1 to threads 1, 2 and 3 | Demo |  | Open |

</details>

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
  cmake --preset stm32f767          # or: qemu, qemu-stress, qemu-ast1030
  cmake --build --preset stm32f767
  ```

  | Preset        | Target                                                  |
  |---------------|---------------------------------------------------------|
  | `stm32f767`   | STM32F767 hardware (Cortex-M7)                          |
  | `qemu`        | QEMU `netduinoplus2` (Cortex-M4F), no hardware needed   |
  | `qemu-stress` | Same as `qemu` with a 10 kHz timeslice to stress preemption |
  | `qemu-ast1030` | QEMU `ast1030-evb` (Aspeed AST1030, Cortex-M4F), which has I2C devices attached |

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

  To run the QEMU regression tests (`tests/qemu`), each a firmware image that reports PASS/FAIL through semihosting:
  ```bash
  cmake --preset qemu
  cmake --build --preset qemu
  ctest --preset qemu          # or qemu-stress for a 10 kHz timeslice
  ```

---

