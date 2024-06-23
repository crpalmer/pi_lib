add_compile_definitions("WIFI_SSID=\"$ENV{WIFI_SSID}\"" "WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"")
add_compile_definitions("CYW43_CONFIG_FILE=\"cyw43_config_pico.h\"")
add_compile_definitions("MG_ARCH=MG_ARCH_CUSTOM")

if("${PLATFORM}" STREQUAL "pico")
  target_sources(lib-pi PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/bluetooth/a2dp-sink.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bluetooth/bluetooth.cpp
    ${CMAKE_CURRENT_LIST_DIR}/wifi.c
  )
endif()

target_include_directories(lib-pi PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/bluetooth
  ${EXTERNAL_DIR}/mongoose/
)


target_sources(lib-pi PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/httpd-filesystem-handler.cpp
  ${CMAKE_CURRENT_LIST_DIR}/httpd-server.cpp
  ${CMAKE_CURRENT_LIST_DIR}/net.c
  ${CMAKE_CURRENT_LIST_DIR}/net-console.cpp
  ${CMAKE_CURRENT_LIST_DIR}/net-line-reader.c
  ${CMAKE_CURRENT_LIST_DIR}/net-listener.cpp
  ${CMAKE_CURRENT_LIST_DIR}/net-reader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/net-writer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/sntp.c
  ${EXTERNAL_DIR}/mongoose/mongoose.c
)
