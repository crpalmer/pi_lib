add_subdirectory("${PLATFORM}")
target_include_directories(pi-threads-platform PUBLIC . )

add_library(pi-threads STATIC
  lights.cpp
)

target_link_libraries(pi-threads
  pi-core
  pi-threads-platform
)
