if("${PLATFORM}" STREQUAL "pico")
  set(PLATFORM_LIBS
    pico_stdlib
  )
else()
  set(PLATFORM_SOURCE
  )
  set(PLATFORM_LIBS
  )
endif()

add_library(pi-audio STATIC
  audio-${PLATFORM}.cpp
  audio-player.cpp
  ${PLATFORM_SOURCE}
  talker-auto-gain.c
  talking-skull.cpp
  talking-skull-from-audio.cpp
  wav.cpp
)

target_include_directories(pi-audio PUBLIC ".")

target_link_libraries(pi-audio PUBLIC
  ${PLATFORM_LIBS}
  pi-core
  pi-threads
)
