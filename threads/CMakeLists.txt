add_library(lib-pi-threads INTERFACE)

target_compile_definitions(lib-pi-threads INTERFACE
  USING_LIBPI_THREADS
)

target_sources(lib-pi-threads INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/digital-counter.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lights.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pi-threads-c.cpp
  ${CMAKE_CURRENT_LIST_DIR}/producer-consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/stop.c
  ${CMAKE_CURRENT_LIST_DIR}/touchscreen-ft6336.cpp
)

target_include_directories(lib-pi-threads INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

include(${PLATFORM_DIR}/platform.cmake)
