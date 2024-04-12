project(net)

include(../platform.cmake)

platform_add_internal_definitions()

if("${PLATFORM}" STREQUAL "pico")
  set(PLATFORM_OBJS
    ble.cpp
  )
  set(PLATFORM_LIBS
    pico_stdlib
  )
elseif("${PLATFORM}" STREQUAL "pi")
  set(PLATFORM_OBJS
    net.c
    net-line-reader.c
    server.c
  )
  set(PLATFORM_LIBS
  )
endif()

add_library(net OBJECT
  ${PLATFORM_OBJS}
)

target_include_directories(net PRIVATE . ..)

target_link_libraries(
    net
    ${PLATFORM_LIBS}
)
