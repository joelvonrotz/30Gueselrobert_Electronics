# check minimum (this is required for top most CMakeLists.txt) 
cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(RecyclingRobert C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set PICO_BOARD to "pico" (default board)
set(ENV{PICO_BOARD} pico)

pico_sdk_init()

# add both 'inc' and 'src' folder to solution, so libraries can be included easily
include_directories(
  inc
  src
)

add_executable(${PROJECT_NAME}
  main.cpp
  src/digitalout.cpp
  src/scara.cpp
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)


pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  pico_stdlib
  hardware_irq
  hardware_i2c
  hardware_adc
  hardware_uart
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)

