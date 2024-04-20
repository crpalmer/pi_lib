if("${PLATFORM}" STREQUAL "pico")
  set(PLATFORM_OBJS
    ble.cpp
    wifi.c
  )
  set(PLATFORM_LIBS
    pico_stdlib
    pico_cyw43_arch_lwip_sys_freertos
  )
elseif("${PLATFORM}" STREQUAL "pi")
  set(PLATFORM_OBJS
  )
  set(PLATFORM_LIBS
  )
endif()

add_library(pi-net STATIC
  net.c
  net-line-reader.c
  net-listener.cpp
  server.c
  sntp.c
  ${PLATFORM_OBJS}
)

add_compile_definitions("WIFI_SSID=\"$ENV{WIFI_SSID}\"" "WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"")

target_include_directories(pi-net PRIVATE ..)
target_include_directories(pi-net PUBLIC .)

target_link_libraries(pi-net PUBLIC
    pi-core
    pi-threads
    ${PLATFORM_LIBS}
)
