target_sources(lib-pi-threads INTERFACE
   ${CMAKE_CURRENT_LIST_DIR}/platform-threads.cpp
)

target_include_directories(lib-pi-threads INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
