add_compile_options(-Wno-error)

if("${PLATFORM}" STREQUAL "pico")
  cmake_policy(SET CMP0076 NEW)
  add_subdirectory(FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI)
else()
  add_subdirectory(tinyalsa)
endif()
