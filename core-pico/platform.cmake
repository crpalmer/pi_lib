target_sources(lib-pi PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/call-every.c
  ${CMAKE_CURRENT_LIST_DIR}/i2c.cpp
  ${CMAKE_CURRENT_LIST_DIR}/light-sensor.cpp
  ${CMAKE_CURRENT_LIST_DIR}/neopixel-pico.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pi.c
  ${CMAKE_CURRENT_LIST_DIR}/pi-gpio-pico.c
  ${CMAKE_CURRENT_LIST_DIR}/pico-adc.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pico-notes.cpp
  ${CMAKE_CURRENT_LIST_DIR}/time-utils.c
  ${CMAKE_CURRENT_LIST_DIR}/uart-reader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/uart-writer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/usb-reader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/usb-writer.cpp
)

pico_generate_pio_header(lib-pi ${CMAKE_CURRENT_LIST_DIR}/neopixel.pio)
