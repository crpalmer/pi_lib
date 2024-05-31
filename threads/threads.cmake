target_sources(lib-pi PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/digital-counter.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lights.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pi-threads-c.cpp
  ${CMAKE_CURRENT_LIST_DIR}/producer-consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/stop.c
)
