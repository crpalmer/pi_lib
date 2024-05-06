add_subdirectory("${PLATFORM}")
target_include_directories(pi-threads-platform PUBLIC . )

add_library(pi-threads STATIC
  digital-counter.cpp
  lights.cpp
  pi-threads-c.cpp
  producer-consumer.c
  stop.c
)

target_link_libraries(pi-threads
  pi-core
  pi-threads-platform
)
