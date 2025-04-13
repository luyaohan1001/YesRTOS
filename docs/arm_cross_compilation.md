# Following flags are used for cross-compilation for ARM platform.

  -mfloat-abi=soft

  To resolve following error due to objects and having inconsistenct float point abi:

  /opt/homebrew/Cellar/arm-none-eabi-gcc/10.3-2021.10/gcc/bin/../lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/bin/ld: error: YesRTOS.elf uses VFP register arguments, CMakeFiles/YesRTOS.dir/src/test.cpp.obj does not

  ----

  --specs=nosys.specs

  To suppress the compiler from looking for syscall symbols:

/opt/homebrew/Cellar/arm-none-eabi-gcc/10.3-2021.10/gcc/bin/../lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/bin/ld: /opt/homebrew/Cellar/arm-none-eabi-gcc/10.3-2021.10/gcc/bin/../lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/lib/thumb/v7e-m+dp/hard/libc_nano.a(lib_a-abort.o): in function `abort':
abort.c:(.text.abort+0xa): undefined reference to `_exit'

/opt/homebrew/Cellar/arm-none-eabi-gcc/10.3-2021.10/gcc/bin/../lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/bin/ld: /opt/homebrew/Cellar/arm-none-eabi-gcc/10.3-2021.10/gcc/bin/../lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/lib/thumb/v7e-m+dp/hard/libc_nano.a(lib_a-sbrkr.o): in function `_sbrk_r':

sbrkr.c:(.text._sbrk_r+0xc): undefined reference to `_sbrk'
/opt/homebrew/Cellar/arm-none-eabi-gcc/10.3-2021.10/gcc/bin/../lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/bin/ld: /opt/homebrew/Cellar/arm-none-eabi-gcc/10.3-2021.10/gcc/bin/../lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/lib/thumb/v7e-m+dp/hard/libc_nano.a(lib_a-signalr.o): in function `_kill_r':

----

# The symbol Reset_Handler should be defined in the startup code, to avoid:

/opt/homebrew/Cellar/arm-none-eabi-gcc/10.3-2021.10/gcc/bin/../lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/bin/ld: warning: cannot find entry symbol Reset_Handler; defaulting to 0000000008000000

# To avoid CMake automatically detect the toolchain for HOST platform.
If it automatically detects, for example MacOS is arm64, it will error out with:
```
arm-none-eabi-g++: error: arm64: No such file or directory
arm-none-eabi-g++: error: unrecognized command-line option '-arch'; did you mean '-march='?
```
To resolve, following switch should be included as part of CMake:
```
# Specify a generic embedded target system and ARM architecture for cross-compilation.
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

# Force CMake to treat specified compilers as valid GNU C/C++ compilers without detection.
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)
```




