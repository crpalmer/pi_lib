add_library(lib-pi INTERFACE)

target_include_directories(lib-pi INTERFACE
  include
  include/${PLATFORM_DIR}
)

target_sources(lib-pi INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/buffer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/canvas.cpp
  ${CMAKE_CURRENT_LIST_DIR}/canvas-impl.cpp
  ${CMAKE_CURRENT_LIST_DIR}/console.cpp
  ${CMAKE_CURRENT_LIST_DIR}/consoles.cpp
  ${CMAKE_CURRENT_LIST_DIR}/core-lock.cpp
  ${CMAKE_CURRENT_LIST_DIR}/file.c
  ${CMAKE_CURRENT_LIST_DIR}/grove.cpp
  ${CMAKE_CURRENT_LIST_DIR}/il9341.cpp
  ${CMAKE_CURRENT_LIST_DIR}/l298n.cpp
  ${CMAKE_CURRENT_LIST_DIR}/image-png.cpp
  ${CMAKE_CURRENT_LIST_DIR}/mcp23017.cpp
  ${CMAKE_CURRENT_LIST_DIR}/mem.c
  ${CMAKE_CURRENT_LIST_DIR}/pca9685.cpp
  ${CMAKE_CURRENT_LIST_DIR}/physics.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ssd1306.cpp
  ${CMAKE_CURRENT_LIST_DIR}/st7735s.cpp
  ${CMAKE_CURRENT_LIST_DIR}/st7796s.cpp
  ${CMAKE_CURRENT_LIST_DIR}/string-utils.c
  ${CMAKE_CURRENT_LIST_DIR}/random-utils.c
)

target_link_libraries(lib-pi INTERFACE
  pngle
)
include(${PLATFORM_DIR}/platform.cmake)
