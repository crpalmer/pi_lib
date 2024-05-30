add_library(pi-threads STATIC)

include(core/core.cmake)
include(${PLATFORM}/platform.cmake)
include(net/net.cmake)

target_link_libraries(pi-threads PUBLIC
  pi-core
)
