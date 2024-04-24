add_library(platform OBJECT
  call-every.c
  i2c.cpp
  light-sensor.cpp
  neopixel-pico.cpp
  pi.c
  pi-gpio-pico.c
  pico-notes.cpp
  pico-servo.c
)

target_include_directories(platform PRIVATE ..)

pico_generate_pio_header(platform ${CMAKE_CURRENT_LIST_DIR}/pico-audio.pio)
pico_generate_pio_header(platform ${CMAKE_CURRENT_LIST_DIR}/neopixel.pio)

target_link_libraries(platform PUBLIC
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_pio
    hardware_pwm
    hardware_rtc
)
