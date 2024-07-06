#### 1) Define toolchain related settings
cmake_minimum_required(VERSION 3.10)

# Include toolchain file
include(gcc-arm-none-eabi.cmake)

enable_language(C CXX ASM)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Build Debug mode for gdb debugging
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()



#### 2) Setup project and source
set(PROJECT_NAME YesRTOS)
project(${PROJECT_NAME} VERSION 1.0)

# Enable compile command dumping to json
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_library(LibYesRTOSTopLayer INTERFACE)
target_sources(LibYesRTOSTopLayer INTERFACE
    ${CMAKE_SOURCE_DIR}/src/scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/thread.cpp)
target_include_directories(LibYesRTOSTopLayer INTERFACE
    ${CMAKE_SOURCE_DIR}/include/)

# Subdirectory source specific to armv7m
add_subdirectory(src/armv7m)



#### 3) Linking - Create executable
add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    LibStartupASM
    LibBareMetal
    LibYesRTOSTopLayer
)

target_include_directories(${PROJECT_NAME} PUBLIC include)