add_compile_options(-Wno-error)

if("${PLATFORM}" STREQUAL "pi")
  add_subdirectory(tinyalsa)
else()
  cmake_policy(SET CMP0076 NEW)
  add_subdirectory(FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI)
endif()
