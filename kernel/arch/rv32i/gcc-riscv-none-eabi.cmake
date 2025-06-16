# This cmake module: cross compilation configuration to compile for RISCV32I (target platform) on MACOS (host).

# Avoids recursive inclusion.
include_guard(GLOBAL)

# Specify a generic embedded target system and ARM architecture for cross-compilation.
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          riscv32)

# # Force CMake to treat specified compilers as valid GNU C/C++ compilers without detection.
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

# Ensure toolchain is installed on macOS via Homebrew if not found.
find_program(RISCV_GCC riscv64-unknown-elf-gcc)

if(NOT RISCV_GCC)
    message(STATUS "riscv64-unknown-elf-gcc not found. Attempting to install via Homebrew...")
    execute_process(
        COMMAND brew install riscv-tools
        RESULT_VARIABLE BREW_RESULT
    )
    if(NOT BREW_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to install riscv-tools using Homebrew.")
    endif()
else()
    message(STATUS "Found arm-none-eabi-gcc: ${RISCV_GCC}")
endif()

# Define tool chain path.
set(TOOLCHAIN_PATH_MACOS            /opt/homebrew/bin)
set(TOOLCHAIN_PREFIX                ${TOOLCHAIN_PATH_MACOS}/riscv64-unknown-elf-)

set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_LINKER                    ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size)

# Set output executable suffix to .elf for ASM, C, and C++ targets.
set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MCU specific flags.
# set(TARGET_FLAGS "-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard")
set(TARGET_FLAGS "-march=rv32imac -mabi=ilp32 -nostartfiles")
set(LINKER_SCRIPT_PATH ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")

# Enable additional C compiler warnings (-Wall, -Wextra, -Wpedantic) and optimize code by placing data and functions in separate sections (-fdata-sections, -ffunction-sections) for better linker garbage collection.
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections")

# Set different optimization and debug flags based on build type: Debug (-O0, -g3) and Release (-Os, -g0).
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
endif()
if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -g0")
endif()

# Set assembler flags to enable preprocessor, generate dependency files (-MMD, -MP) for ASM files.
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")

# Set C++ compiler flags to disable RTTI, exceptions, and thread-safe statics for reduced binary size and performance in embedded systems.
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_C_LINK_FLAGS "${TARGET_FLAGS}")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T \"${LINKER_SCRIPT_PATH}/riscv_qemu_virt.ld\"")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-Map=memory_dump.map -Wl,--gc-sections")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lc -lm -Wl,--end-group")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--print-memory-usage")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lstdc++ -lsupc++ -Wl,--end-group")

# Enable verbose link.
# set(CMAKE_CXX_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--verbose")
