add_library(pi-threads-platform STATIC
   pi-threads.cpp
)

target_include_directories(pi-threads-platform PUBLIC . )

target_link_libraries(pi-threads-platform PUBLIC
   pi-core
   pthread
)
