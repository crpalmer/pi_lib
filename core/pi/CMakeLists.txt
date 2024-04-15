add_library(platform OBJECT
  audio.c
  call-every.c
  canvas-png.cpp
  digital-counter.cpp
  i2c.cpp
  maestro.c
  neopixel-pi.cpp
  nes.c
  pi.c
  pi-gpio-pi.c
  pi-gpio-pi-servo.c
  pi-usb.c
  pico-slave.cpp
  producer-consumer.c
  st7735s.cpp
  stop.c
  talker-auto-gain.c
  talking-skull.c
  track.c
  wav.c
)

add_definitions(-DNO_PIGPIO_EMULATION -DWall -Werror)

target_include_directories(platform PRIVATE . ..)

target_link_libraries(platform
    tinyalsa
)
