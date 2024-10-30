target_sources(lib-pi PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/call-every.c
  ${CMAKE_CURRENT_LIST_DIR}/canvas-png.cpp
  ${CMAKE_CURRENT_LIST_DIR}/file.cpp
  ${CMAKE_CURRENT_LIST_DIR}/maestro.c
  ${CMAKE_CURRENT_LIST_DIR}/neopixel-pi.cpp
  ${CMAKE_CURRENT_LIST_DIR}/nes.c
  ${CMAKE_CURRENT_LIST_DIR}/pi.c
  ${CMAKE_CURRENT_LIST_DIR}/pi-gpio-pi.c
  ${CMAKE_CURRENT_LIST_DIR}/pi-gpio-pi-servo.c
  ${CMAKE_CURRENT_LIST_DIR}/pi-usb.c
  ${CMAKE_CURRENT_LIST_DIR}/pico-slave.cpp
  ${CMAKE_CURRENT_LIST_DIR}/spi.cpp
  ${CMAKE_CURRENT_LIST_DIR}/st7735s.cpp
  ${CMAKE_CURRENT_LIST_DIR}/time-utils.c
)

target_compile_definitions(lib-pi PRIVATE NO_PIGPIO_EMULATION Wall Werror)

target_link_libraries(lib-pi PRIVATE
    tinyalsa
)
