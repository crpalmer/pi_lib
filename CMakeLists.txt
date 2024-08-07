cmake_minimum_required(VERSION 3.12)

if("${PLATFORM}" STREQUAL pico)
  include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
  #include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
endif()

add_subdirectory(external)

project(pi-lib)

if("${PLATFORM}" STREQUAL pico)
  pico_sdk_init()
endif()

include(platform.cmake)

set(EXTERNAL_DIR "${CMAKE_CURRENT_LIST_DIR}/external")

add_library(lib-pi STATIC)

target_compile_definitions(lib-pi PRIVATE LIB_PI_INTERNAL)

if(${PLATFORM} STREQUAL pico)
  SDK_INTERFACE_LIBS(lib-pi
    pico_runtime
    pico_stdio
    pico_stdio_uart
    pico_stdio_usb
    hardware_watchdog
    pico_binary_info
  )
  target_link_libraries(lib-pi PRIVATE
    pico_stdlib
    pico_cyw43_arch_lwip_sys_freertos
    pico_btstack_ble
    pico_btstack_classic
    pico_btstack_cyw43
    pico_btstack_sbc_decoder
    pico_btstack_sbc_encoder
    hardware_adc
    hardware_dma
    hardware_i2c
    hardware_pio
    hardware_pio
    hardware_pwm
    hardware_rtc
  )
else()
  target_link_libraries(lib-pi PUBLIC pthread usb rt pthread png16 gpiod)
  if ("${PLATFORM}" STREQUAL pi)
    target_link_libraries(lib-pi PUBLIC pigpio)
  endif()
endif()

target_include_directories(lib-pi
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/include/${PLATFORM_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include/threads-${PLATFORM_DIR}
)

include(audio/audio.cmake)
include(core/core.cmake)
include(core-${PLATFORM_DIR}/platform.cmake)
include(net/net.cmake)
include(threads/threads.cmake)
include(threads-${PLATFORM_DIR}/platform.cmake)

add_subdirectory(utils)
