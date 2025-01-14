add_library(lib-pi-audio INTERFACE)

target_sources(lib-pi-audio INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/audio-${PLATFORM_DIR}.cpp
  ${CMAKE_CURRENT_LIST_DIR}/audio-player.cpp
  ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SOURCE}
  ${CMAKE_CURRENT_LIST_DIR}/talker-auto-gain.c
  ${CMAKE_CURRENT_LIST_DIR}/talking-skull.cpp
  ${CMAKE_CURRENT_LIST_DIR}/talking-skull-from-audio.cpp
  ${CMAKE_CURRENT_LIST_DIR}/wav.cpp
)

if("${PLATFORM}" STREQUAL "pico")
  pico_generate_pio_header(lib-pi-audio ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.pio)
endif()

target_include_directories(lib-pi-audio INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
