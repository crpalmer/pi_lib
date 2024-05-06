cmake_minimum_required(VERSION 3.12)

if("${PLATFORM}" STREQUAL pico)
#  set(PICO_BOARD pico_w)
  include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
  include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
endif()

add_subdirectory(external)

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
