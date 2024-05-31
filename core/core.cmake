target_sources(lib-pi PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/buffer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/canvas.cpp
  ${CMAKE_CURRENT_LIST_DIR}/console.cpp
  ${CMAKE_CURRENT_LIST_DIR}/consoles.cpp
  ${CMAKE_CURRENT_LIST_DIR}/file.c
  ${CMAKE_CURRENT_LIST_DIR}/grove.cpp
  ${CMAKE_CURRENT_LIST_DIR}/l298n.cpp
  ${CMAKE_CURRENT_LIST_DIR}/mcp23017.cpp
  ${CMAKE_CURRENT_LIST_DIR}/mem.c
  ${CMAKE_CURRENT_LIST_DIR}/pca9685.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ssd1306.cpp
  ${CMAKE_CURRENT_LIST_DIR}/string-utils.c
  ${CMAKE_CURRENT_LIST_DIR}/random-utils.c
)

if("${PLATFORM}" STREQUAL "pico")
    target_link_libraries(lib-pi PRIVATE pico_stdlib)
else()
    target_link_libraries(lib-pi PUBLIC usb rt pthread png16 gpiod pigpio)
endif()

