cmake_minimum_required(VERSION 3.12)

if("${PLATFORM}" STREQUAL pico)
  include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
  include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
endif()

project(pi-lib)

if("${PLATFORM}" STREQUAL pico)
  pico_sdk_init()
endif()

include(platform.cmake)

add_subdirectory(audio)
add_subdirectory(core)
add_subdirectory(net)
add_subdirectory(threads)
add_subdirectory(utils)

if("${PLATFORM}" STREQUAL "pi")
  add_subdirectory(tinyalsa)
endif()
