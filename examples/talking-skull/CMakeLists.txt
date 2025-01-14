util_cpp(talking-skull)
target_link_libraries(talking-skull PRIVATE lib-pi-audio lib-pi-threads)

util_cpp(talking-skull-laugh)
target_link_libraries(talking-skull-laugh PRIVATE lib-pi-audio lib-pi-threads)

if("${PLATFORM_dir}" STREQUAL pi)
   util_cpp(talking-skull-dump)
   target_link_libraries(talking-skull-dump PRIVATE lib-pi-audio lib-pi-threads)
endif()
