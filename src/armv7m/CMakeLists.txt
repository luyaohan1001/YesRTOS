#### 1) Project source settings.
set(PROJECT_NAME YesRTOSMachineLayer)
project(${PROJECT_NAME})

# Add first library including startup code in assembly.
# Compile assembly to its own file so debugger could locate this symbol.
add_library(LibStartupASM
    ./startup_stm32f767xx.s
)

# Add second library which includes all other bare metal code.
add_library(LibBareMetal INTERFACE)
target_compile_definitions(LibBareMetal INTERFACE
	USE_HAL_DRIVER
	STM32F767xx
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(LibBareMetal INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/Core/Inc
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Include
)

target_sources(LibBareMetal INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/baremetal.c
    ${CMAKE_CURRENT_LIST_DIR}/Core/Src/stm32f7xx_it.c
    ${CMAKE_CURRENT_LIST_DIR}/Core/Src/stm32f7xx_hal_msp.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart_ex.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_exti.c
    ${CMAKE_CURRENT_LIST_DIR}/Core/Src/system_stm32f7xx.c
    ${CMAKE_CURRENT_LIST_DIR}/Core/Src/sysmem.c
    ${CMAKE_CURRENT_LIST_DIR}/Core/Src/syscalls.c
)



#### 2) Linking - Create executable
add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    LibStartupASM
    LibBareMetal
)



#### 3) Custom commands (utilities and tools)

# Flash command.
add_custom_target(flash
    COMMAND openocd
            -f interface/stlink-v2-1.cfg
            -f target/stm32f7x.cfg
            -c 'program ${CMAKE_SOURCE_DIR}/build/src/armv7m/${PROJECT_NAME}.elf verify reset exit'
)

# Disassemble machine layer code.
add_custom_target(dump
    COMMAND arm-none-eabi-objdump -d ${CMAKE_SOURCE_DIR}/build/src/armv7m/${PROJECT_NAME} > ${CMAKE_SOURCE_DIR}/build/src/armv7m/${PROJECT_NAME}.dump
)