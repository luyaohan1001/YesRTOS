# Multi-thread demo application. Added from the top-level CMakeLists.txt; the toolchain comes from CMAKE_TOOLCHAIN_FILE.

set(PROJECT_NAME multi_thread)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)

if (ARCH_DEFINED STREQUAL "ARMV7M")
  target_compile_definitions(${PROJECT_NAME} PRIVATE ARMV7M)
endif()

# Header include directory.
target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/../../kernel/include
)

if (ARCH_DEFINED STREQUAL "ARMV7M")
  target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../kernel/arch/armv7m
  )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
  LibYesRTOSKernel
  LibBareMetal
)

# Memory map next to the ELF.
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Map=$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.map)

# Kill debug session
add_custom_target(kill
COMMAND pkill openocd
)

# Flash command.
add_custom_target(flash
COMMAND openocd
            -f interface/stlink-v2-1.cfg
            -f target/stm32f7x.cfg
            -c 'program $<TARGET_FILE:${PROJECT_NAME}> verify reset exit'
DEPENDS ${PROJECT_NAME}
)

# Dump symbol table using nm util.
add_custom_target(dump_symbol_table
        ${CMAKE_NM}
        --size-sort
        --print-size
        --numeric-sort
        --line-numbers
        --radix=x
        $<TARGET_FILE:${PROJECT_NAME}> > symtable.dump
DEPENDS ${PROJECT_NAME}
)

# Dump size from ELF (executable and linkable format) file.
add_custom_target(dump_section_size
        ${CMAKE_SIZE} -A
        $<TARGET_FILE:${PROJECT_NAME}> > size.dump
DEPENDS ${PROJECT_NAME}
)
