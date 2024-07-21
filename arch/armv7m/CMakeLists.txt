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
    ${CMAKE_CURRENT_LIST_DIR}   # ${CMAKE_CURRENT_LIST_DIR} maps to current folder: arch/armv7m
)

target_sources(LibBareMetal INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/trace.c
    ${CMAKE_CURRENT_LIST_DIR}/timeslice.cpp
)