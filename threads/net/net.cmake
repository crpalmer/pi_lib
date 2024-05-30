add_compile_definitions("WIFI_SSID=\"$ENV{WIFI_SSID}\"" "WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"")

if("${PLATFORM}" STREQUAL "pico")
  target_sources(pi-threads PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/wifi.c
  )
  target_link_libraries(pi-threads PUBLIC
    pico_cyw43_arch_lwip_sys_freertos
  )
endif()

target_sources(pi-threads PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/net.c
  ${CMAKE_CURRENT_LIST_DIR}/net-line-reader.c
  ${CMAKE_CURRENT_LIST_DIR}/net-listener.cpp
  ${CMAKE_CURRENT_LIST_DIR}/sntp.c
)

target_include_directories(pi-threads PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
