add_library(lib-pi-net INTERFACE)

target_compile_definitions(lib-pi-net INTERFACE "WIFI_SSID=\"$ENV{WIFI_SSID}\"" "WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"")
target_compile_definitions(lib-pi-net INTERFACE "CYW43_CONFIG_FILE=\"cyw43_config_pico.h\"")
target_compile_definitions(lib-pi-net INTERFACE "MG_ARCH=MG_ARCH_CUSTOM")

if("${PLATFORM}" STREQUAL "pico")
  include("${CMAKE_CURRENT_LIST_DIR}/bluetooth/bluetooth.cmake")

  target_sources(lib-pi-net INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pico-battery.c
    ${CMAKE_CURRENT_LIST_DIR}/wifi.c
  )

  target_link_libraries(lib-pi-net INTERFACE
    pico_cyw43_arch_lwip_sys_freertos
  )
endif()

target_include_directories(lib-pi-net INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/bluetooth
  ${EXTERNAL_DIR}/mongoose/
)

target_sources(lib-pi-net INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/httpd-filesystem-handler.cpp
  ${CMAKE_CURRENT_LIST_DIR}/httpd-server.cpp
  ${CMAKE_CURRENT_LIST_DIR}/net.c
  ${CMAKE_CURRENT_LIST_DIR}/net-console.cpp
  ${CMAKE_CURRENT_LIST_DIR}/net-listener.cpp
  ${CMAKE_CURRENT_LIST_DIR}/net-reader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/net-writer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/sntp.c
  ${EXTERNAL_DIR}/mongoose/mongoose.c
)

target_include_directories(lib-pi-net INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
