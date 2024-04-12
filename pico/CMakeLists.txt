project(platform_pico)

include(../platform.cmake)

add_library(platform_pico OBJECT
  call-every.c
  i2c.cpp
  light-sensor.cpp
  neopixel-pico.cpp
  pi.c
  pi-gpio-pico.c
  pico-audio.cpp
  pico-notes.cpp
  pico_servo.c
)

platform_add_internal_definitions()

target_include_directories(platform_pico PRIVATE ..)

pico_generate_pio_header(platform_pico ${CMAKE_CURRENT_LIST_DIR}/pico-audio.pio)
pico_generate_pio_header(platform_pico ${CMAKE_CURRENT_LIST_DIR}/neopixel.pio)

target_link_libraries(
    platform_pico
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_pio
    hardware_pwm
)
