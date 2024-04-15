add_library(pi-threads-platform STATIC
   pi-threads.c
)

target_link_libraries(pi-threads-platform PUBLIC
   pi-core
   pthread
)
