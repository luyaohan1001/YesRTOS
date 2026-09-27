# This cmake toolchain file: cross compilation configuration to compile for ARM-v7m (target platform) on MACOS (host).
# Passed once via CMAKE_TOOLCHAIN_FILE (see CMakePresets.json), so it is read before project() enables languages.

# Specify a generic embedded target system and ARM architecture for cross-compilation.
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

# # Force CMake to treat specified compilers as valid GNU C/C++ compilers without detection.
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

# Ensure arm-none-eabi toolchain is installed on macOS via Homebrew if not found.
find_program(
    ARM_GCC
    NAMES arm-none-eabi-gcc
    PATHS ENV PATH
)

if (NOT ARM_GCC)
    message(FATAL_ERROR "Failed to find arm-none-eabi-gcc compiler binary. Please install and add executable path to environment variable.")
endif()

# Define tool chain path.
get_filename_component(TOOLCHAIN_BIN_DIR ${ARM_GCC} DIRECTORY)
set(TOOLCHAIN_PREFIX                ${TOOLCHAIN_BIN_DIR}/arm-none-eabi-)

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

# MCU specific flags for the BOARD cache variable (set by the preset; see the top-level CMakeLists.txt).
if (NOT BOARD OR BOARD STREQUAL "stm32f767")
  set(TARGET_FLAGS "-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard")      # STM32F767: Cortex-M7, FPv5 double precision
else()
  set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard")   # QEMU netduinoplus2 and ast1030: Cortex-M4F
endif()

# The *_INIT variables only seed the defaults, so flags given on the command line (e.g. -DCMAKE_CXX_FLAGS=...) are appended rather than replacing these.
# Enable additional C compiler warnings (-Wall, -Wextra, -Wpedantic) and optimize code by placing data and functions in separate sections (-fdata-sections, -ffunction-sections) for better linker garbage collection.
set(CMAKE_C_FLAGS_INIT              "${TARGET_FLAGS} -Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections")

# Set assembler flags to enable preprocessor, generate dependency files (-MMD, -MP) for ASM files.
set(CMAKE_ASM_FLAGS_INIT            "${CMAKE_C_FLAGS_INIT} -x assembler-with-cpp -MMD -MP")

# Set C++ compiler flags to disable RTTI, exceptions, and thread-safe statics for reduced binary size and performance in embedded systems.
set(CMAKE_CXX_FLAGS_INIT            "${CMAKE_C_FLAGS_INIT} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

# Set different optimization and debug flags based on build type: Debug (-O0, -g3) and Release (-Os, -g0).
# Cached rather than *_INIT, since CMake's GNU compiler module appends its own "-g" / "-O3 -DNDEBUG" to the *_INIT values.
foreach(LANG C CXX ASM)
  set(CMAKE_${LANG}_FLAGS_DEBUG    "-O0 -g3" CACHE STRING "Flags used by the ${LANG} compiler during Debug builds.")
  set(CMAKE_${LANG}_FLAGS_RELEASE  "-Os -g0" CACHE STRING "Flags used by the ${LANG} compiler during Release builds.")
endforeach()

# Linker script is attached by the board's LibBareMetal target (kernel/arch/armv7m/CMakeLists.txt).
set(CMAKE_EXE_LINKER_FLAGS_INIT     "-Wl,--gc-sections -Wl,--print-memory-usage")
set(CMAKE_C_STANDARD_LIBRARIES_INIT   "-Wl,--start-group -lc -lm -Wl,--end-group")
set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "-Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group")
