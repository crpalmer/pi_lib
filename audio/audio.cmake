target_sources(lib-pi PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/audio-${PLATFORM_DIR}.cpp
  ${CMAKE_CURRENT_LIST_DIR}/audio-player.cpp
  ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SOURCE}
  ${CMAKE_CURRENT_LIST_DIR}/talker-auto-gain.c
  ${CMAKE_CURRENT_LIST_DIR}/talking-skull.cpp
  ${CMAKE_CURRENT_LIST_DIR}/talking-skull-from-audio.cpp
  ${CMAKE_CURRENT_LIST_DIR}/wav.cpp
)

if("${PLATFORM}" STREQUAL "pico")
  pico_generate_pio_header(lib-pi ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.pio)
endif()
