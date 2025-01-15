if ("${PICO_BOARD}" STREQUAL "pico" OR "${PICO_BOARD}" STREQUAL "pico_w")
  target_compile_definitions(lib-pi INTERFACE HAVE_RTC)
  target_link_libraries(lib-pi INTERFACE
    hardware_rtc
  )
endif()

target_include_directories(lib-pi INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(lib-pi INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/call-every.c
  ${CMAKE_CURRENT_LIST_DIR}/deep-sleep.c
  ${CMAKE_CURRENT_LIST_DIR}/distance.cpp
  ${CMAKE_CURRENT_LIST_DIR}/i2c.cpp
  ${CMAKE_CURRENT_LIST_DIR}/light-sensor.cpp
  ${CMAKE_CURRENT_LIST_DIR}/neopixel-pico.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pi.c
  ${CMAKE_CURRENT_LIST_DIR}/pi-gpio-pico.c
  ${CMAKE_CURRENT_LIST_DIR}/pico-adc.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pico-notes.cpp
  ${CMAKE_CURRENT_LIST_DIR}/spi.cpp
  ${CMAKE_CURRENT_LIST_DIR}/time-utils.c
  ${CMAKE_CURRENT_LIST_DIR}/uart-reader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/uart-writer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/usb-reader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/usb-writer.cpp
)

pico_generate_pio_header(lib-pi ${CMAKE_CURRENT_LIST_DIR}/neopixel.pio)

target_link_libraries(lib-pi INTERFACE
  pico_stdio
  pico_stdio_usb
  pico_stdlib
  hardware_adc
  hardware_i2c
  hardware_pio
  hardware_pwm
  hardware_sleep
  hardware_spi
)
