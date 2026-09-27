# YesRTOS drivers: the board independent I2C interface and device drivers, plus the controller driver of the board.

add_library(LibYesRTOSDrivers STATIC
  ${CMAKE_CURRENT_LIST_DIR}/i2c/i2c_bus.cpp
  ${CMAKE_CURRENT_LIST_DIR}/devices/tmp105.cpp
  ${CMAKE_CURRENT_LIST_DIR}/devices/smbus_eeprom.cpp
)

# I2C controller of the board, if it has one QEMU emulates.
if (BOARD STREQUAL "ast1030")
  target_sources(LibYesRTOSDrivers PRIVATE ${CMAKE_CURRENT_LIST_DIR}/i2c/aspeed_i2c.cpp)
endif()

target_include_directories(LibYesRTOSDrivers PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/i2c
  ${CMAKE_CURRENT_LIST_DIR}/devices
)
target_link_libraries(LibYesRTOSDrivers PUBLIC LibYesRTOSKernel)
