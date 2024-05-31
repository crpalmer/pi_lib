target_sources(lib-pi PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/pi-threads.cpp
)

target_link_libraries(lib-pi PUBLIC
   pthread
)
