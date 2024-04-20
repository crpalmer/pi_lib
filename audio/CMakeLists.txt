if("${PLATFORM}" STREQUAL "pico")
  set(PLATFORM_LIBS
    pico_stdlib
  )
else()
  set(PLATFORM_SOURCE
    talking-skull.c
  )
  set(PLATFORM_LIBS
  )
endif()

add_library(pi-audio STATIC
  audio-${PLATFORM}.c
  ${PLATFORM_SOURCE}
  talker-auto-gain.c
  track.c
  wav.c
)

target_include_directories(pi-audio PUBLIC ".")

target_link_libraries(pi-audio PUBLIC
  ${PLATFORM_LIBS}
  pi-core
  pi-threads
)
