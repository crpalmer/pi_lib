set(BTSTACK_3RD_PARTY_PATH "/home/crpalmer/pico/pico-sdk/lib/btstack/3rd-party")
target_sources(lib-pi PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/a2dp-sink.cpp
  ${CMAKE_CURRENT_LIST_DIR}/avrcp.cpp
  ${CMAKE_CURRENT_LIST_DIR}/avrcp-connection.cpp
  ${CMAKE_CURRENT_LIST_DIR}/bluetooth.cpp
  ${CMAKE_CURRENT_LIST_DIR}/hid.cpp
  ${CMAKE_CURRENT_LIST_DIR}/sbc-configuration.cpp
  ${CMAKE_CURRENT_LIST_DIR}/sbc-decoder.cpp
  ${CMAKE_CURRENT_LIST_DIR}/junk.cpp
)
