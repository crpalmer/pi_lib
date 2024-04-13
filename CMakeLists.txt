cmake_minimum_required(VERSION 3.12)

include(platform.cmake)

project(pi)

platform_add_internal_definitions()

add_subdirectory(${PLATFORM})
add_subdirectory(net)

if(${PI_IS_TOP_LEVEL})
  add_subdirectory(utils)
endif()

if("${PLATFORM}" STREQUAL "pi")
  add_subdirectory(tinyalsa)
endif()

add_library(pi STATIC
  canvas.cpp
  file.c
  global-trace.c
  grove.cpp
  l298n.cpp
  mcp23017.cpp
  mem.c
  pca9685.cpp
  ssd1306.cpp
  string-utils.c
  util.c
)

if("${PLATFORM}" STREQUAL "pico")
    set(PLATFORM_LIBS
	platform_pico
    )
else()
    set(PLATFORM_LIBS
      platform_pi
      usb rt pthread png16 gpiod pigpio
    )
endif()

target_include_directories(pi PUBLIC "." "${PLATFORM}" net)

target_link_libraries(
    pi
    ${PLATFORM_LIBS}
    net
)
