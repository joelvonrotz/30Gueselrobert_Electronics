# check minimum 
cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(pren_picominator C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set PICO_BOARD to "pico" (default board)
set(ENV{PICO_BOARD} pico)

pico_sdk_init()

include_directories(
  lib
)

add_executable(${PROJECT_NAME}
  main.cpp
  lib/digitalout.cpp
  scara.cpp
)


pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  pico_stdlib
  hardware_irq
  hardware_i2c
  hardware_adc
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
