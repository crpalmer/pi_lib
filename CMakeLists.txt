cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)

include(pico_sdk_import.cmake)

project(pi)

pico_sdk_init()

add_subdirectory(utils)
add_definitions(-DPI_PICO -Iexternal/PIGPIO)

add_library(pi
    ble.cpp
    call-every.c
    canvas.cpp
    file.c
    global-trace.c
    gpio.c
    grove.cpp
    i2c.cpp
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
    hardware_i2c
#    hardware_pwm
#    hardware_adc
    hardware_pio
#    hardware_dma
)
