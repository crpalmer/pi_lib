cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)

include(pico_sdk_import.cmake)

project(pi)

pico_sdk_init()

add_definitions(-DPI_PICO -Iexternal/PIGPIO)

add_library(pi
    call-every.c
    file.c
    global-trace.c
    gpio.c
    l298n.cpp
    mcp23017.cpp
    mem.c
    nes.c
    pca9685.cpp
    pi.c
    pico_servo.c
    pigpio-emulation.c
    stepper.c
    string-utils.c
    util.c
)

target_link_libraries(pi pico_stdlib hardware_pwm)
