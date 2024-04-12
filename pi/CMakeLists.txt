project(platform_pi)

add_library(platform_pi OBJECT
  audio.c
  call-every.c
  canvas_png.cpp
  digital-counter.cpp
  i2c.cpp
  lights.cpp
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
  wb.c
  wav.c
  ween-board.cpp
)

add_definitions(-DNO_PIGPIO_EMULATION -DWall -Werror)

target_include_directories(platform_pi PRIVATE . ..)

target_link_libraries(
    platform_pi
    tinyalsa
)
