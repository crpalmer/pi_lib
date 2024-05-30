target_include_directories(pi-threads PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include )

target_sources(pi-threads PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/digital-counter.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lights.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pi-threads-c.cpp
  ${CMAKE_CURRENT_LIST_DIR}/producer-consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/stop.c
)
