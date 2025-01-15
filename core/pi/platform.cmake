target_sources(lib-pi INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/call-every.c
  ${CMAKE_CURRENT_LIST_DIR}/canvas-png.cpp
  ${CMAKE_CURRENT_LIST_DIR}/file.cpp
  ${CMAKE_CURRENT_LIST_DIR}/i2c.cpp
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

target_compile_definitions(lib-pi INTERFACE NO_PIGPIO_EMULATION Wall Werror)

target_include_directories(lib-pi INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(lib-pi INTERFACE
    tinyalsa
    -lgpiod
    -lpigpio
    -lpng
    -lusb
)
