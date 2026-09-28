# Stack monitor demo: worker threads whose stack use rises and falls, drawn live as bars by YesRTOS::StackMonitor.
# Added from the top-level CMakeLists.txt; the toolchain comes from CMAKE_TOOLCHAIN_FILE.
#
#   cmake --preset qemu && cmake --build --preset qemu --target qemu-stack-monitor

add_executable(stack_monitor main.cpp)
target_link_libraries(stack_monitor PRIVATE LibYesRTOSKernel LibBareMetal)
target_link_options(stack_monitor PRIVATE -Wl,-Map=$<TARGET_FILE_DIR:stack_monitor>/stack_monitor.map)

if (YESRTOS_QEMU_MACHINE)
  # Run in QEMU; the monitor redraws in place on the terminal (exit with Ctrl-A X).
  add_custom_target(qemu-stack-monitor
  COMMAND qemu-system-arm
              -M ${YESRTOS_QEMU_MACHINE}
              -nographic
              -kernel $<TARGET_FILE:stack_monitor>
  DEPENDS stack_monitor
  USES_TERMINAL
  )
endif()
