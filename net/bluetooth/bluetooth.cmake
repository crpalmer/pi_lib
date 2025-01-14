set(BTSTACK_3RD_PARTY_PATH "/home/crpalmer/pico/pico-sdk/lib/btstack/3rd-party")
target_sources(lib-pi-net INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/a2dp-sink.cpp
  ${CMAKE_CURRENT_LIST_DIR}/avrcp.cpp
  ${CMAKE_CURRENT_LIST_DIR}/avrcp-connection.cpp
  ${CMAKE_CURRENT_LIST_DIR}/bluetooth.cpp
  ${CMAKE_CURRENT_LIST_DIR}/hid.cpp
  ${CMAKE_CURRENT_LIST_DIR}/sbc-configuration.cpp
  ${CMAKE_CURRENT_LIST_DIR}/sbc-decoder.cpp
  ${CMAKE_CURRENT_LIST_DIR}/junk.cpp
)

target_include_directories(lib-pi-net INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(lib-pi-net INTERFACE
   pico_btstack_sbc_decoder
   pico_btstack_sbc_encoder
   pico_btstack_ble
   pico_btstack_classic
   pico_btstack_cyw43
   lib-pi-audio
)
