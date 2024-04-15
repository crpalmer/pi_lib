cmake_minimum_required(VERSION 3.12)

include(platform.cmake)

project(pi-lib)

add_subdirectory(core)
add_subdirectory(net)
add_subdirectory(threads)
add_subdirectory(utils)

if("${PLATFORM}" STREQUAL "pi")
  add_subdirectory(tinyalsa)
endif()
