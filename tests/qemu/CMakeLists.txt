# QEMU regression tests. Each test is a firmware image run on the board's QEMU machine; it reports its result
# through semihosting SYS_EXIT, which becomes QEMU's exit status (see test_support.hpp).
#
#   cmake --preset qemu && cmake --build --preset qemu && ctest --preset qemu

set(YESRTOS_QEMU_TESTS
  mutex_exclusion
  cas_spinlock
  critical_section_nesting
  stack_alignment
  exception_priority
  idle_when_all_blocked
  thread_exit
  add_thread
  mutex_states
  heap
  linkedlist
  scheduler_fifo
  startup_pending_switch
  external_interrupt
  semaphore
  semaphore_isr
  tick_hook
)

# Boards whose QEMU machine has I2C devices attached.
if (BOARD STREQUAL "ast1030")
  list(APPEND YESRTOS_QEMU_TESTS
    i2c_eeprom
    i2c_tmp105
    i2c_concurrent
    i3c_daa
    i3c_transfer
    i3c_concurrent
  )
  # Extra QEMU arguments per test: mock I3C targets. QEMU does not arbitrate between several unaddressed targets
  # during dynamic address assignment, so each controller carries at most one.
  set(QEMU_ARGS_i3c_daa
    -device mock-i3c-target,bus=dw.i3c.0,pid=0x04a100001001,bcr=0x06,dcr=0x55
    -device mock-i3c-target,bus=dw.i3c.1,pid=0x04a100002002,bcr=0x07,dcr=0x66)
  set(QEMU_ARGS_i3c_transfer
    -device mock-i3c-target,bus=dw.i3c.0,pid=0x04a100001001,bcr=0x06,dcr=0x55)
  set(QEMU_ARGS_i3c_concurrent ${QEMU_ARGS_i3c_transfer})
endif()

foreach(TEST_NAME ${YESRTOS_QEMU_TESTS})
  set(TEST_TARGET test_${TEST_NAME})
  add_executable(${TEST_TARGET} ${CMAKE_CURRENT_LIST_DIR}/${TEST_NAME}.cpp)
  target_compile_definitions(${TEST_TARGET} PRIVATE ARMV7M)
  target_include_directories(${TEST_TARGET} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/arch/armv7m
  )
  target_link_libraries(${TEST_TARGET} PRIVATE LibYesRTOSKernel LibBareMetal LibYesRTOSDrivers)

  add_test(NAME ${TEST_NAME}
    COMMAND qemu-system-arm
      -M ${YESRTOS_QEMU_MACHINE}
      -nographic
      -monitor none
      -semihosting-config enable=on,target=native
      ${QEMU_ARGS_${TEST_NAME}}
      -kernel $<TARGET_FILE:${TEST_TARGET}>
  )
  # A faulting test spins in Default_Handler; the timeout turns that into a failure.
  set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 10)
endforeach()
