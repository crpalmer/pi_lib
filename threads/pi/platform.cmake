target_sources(pi-threads PUBLIC
   ${CMAKE_CURRENT_LIST_DIR}/pi-threads.cpp
)

target_include_directories(pi-threads PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include )

target_link_libraries(pi-threads PUBLIC
   pthread
)
