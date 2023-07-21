cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

# Pull in FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(pi)

pico_sdk_init()

target_include_directories(FreeRTOS-Kernel INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(utils)
add_subdirectory(utils-freertos)
add_definitions(-DPI_PICO -Iexternal/PIGPIO)

add_library(pi
    ble.cpp
    call-every.c
    file.c
    global-trace.c
    gpio.c
    l298n.cpp
    light-sensor.cpp
    mcp23017.cpp
    mem.c
    neopixel-pico.cpp
    nes.c
    pca9685.cpp
    pi.c
    pico_servo.c
    pico-audio.cpp
    pigpio-emulation.c
    stepper.c
    string-utils.c
    util.c
)

pico_generate_pio_header(pi ${CMAKE_CURRENT_LIST_DIR}/pico-audio.pio)
pico_generate_pio_header(pi ${CMAKE_CURRENT_LIST_DIR}/neopixel.pio)

target_link_libraries(
    pi
    pico_stdlib
    hardware_adc
    hardware_pwm
#    hardware_spi
#    hardware_i2c
#    hardware_pwm
#    hardware_adc
    hardware_pio
#    hardware_dma
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)
