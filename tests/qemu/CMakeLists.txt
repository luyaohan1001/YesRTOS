# QEMU regression tests. Each test is a firmware image run on the netduinoplus2 machine; it reports its result
# through semihosting SYS_EXIT, which becomes QEMU's exit status (see test_support.hpp).
#
#   cmake --preset qemu && cmake --build --preset qemu && ctest --preset qemu

set(YESRTOS_QEMU_TESTS
  mutex_exclusion
  cas_spinlock
  critical_section_nesting
)

foreach(TEST_NAME ${YESRTOS_QEMU_TESTS})
  set(TEST_TARGET test_${TEST_NAME})
  add_executable(${TEST_TARGET} ${CMAKE_CURRENT_LIST_DIR}/${TEST_NAME}.cpp)
  target_compile_definitions(${TEST_TARGET} PRIVATE ARMV7M)
  target_include_directories(${TEST_TARGET} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/arch/armv7m
  )
  target_link_libraries(${TEST_TARGET} PRIVATE LibYesRTOSKernel LibBareMetal)

  add_test(NAME ${TEST_NAME}
    COMMAND qemu-system-arm
      -M netduinoplus2
      -nographic
      -monitor none
      -semihosting-config enable=on,target=native
      -kernel $<TARGET_FILE:${TEST_TARGET}>
  )
  # A faulting test spins in Default_Handler; the timeout turns that into a failure.
  set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 30)
endforeach()
