add_library(lib-pi-threads INTERFACE)

target_sources(lib-pi-threads INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/digital-counter.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lights.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pi-threads-c.cpp
  ${CMAKE_CURRENT_LIST_DIR}/producer-consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/stop.c
)

target_include_directories(lib-pi-threads INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

include(${PLATFORM_DIR}/platform.cmake)
