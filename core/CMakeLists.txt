add_subdirectory("${PLATFORM}")

add_library(pi-core STATIC
  buffer.cpp
  canvas.cpp
  console.cpp
  consoles.cpp
  file.c
  global-trace.c
  grove.cpp
  l298n.cpp
  mcp23017.cpp
  mem.c
  pca9685.cpp
  ssd1306.cpp
  string-utils.c
  random-utils.c
  wb.c
  ween-board.cpp
)

if("${PLATFORM}" STREQUAL "pico")
    set(PLATFORM_LIBS
	pico_stdlib
    )
else()
    set(PLATFORM_LIBS
      usb rt pthread png16 gpiod pigpio
    )
endif()

message(pi-core PUBLIC "." "${PLATFORM}")
target_include_directories(pi-core PUBLIC "." "${PLATFORM}")

target_link_libraries(pi-core PUBLIC
  ${PLATFORM_LIBS}
  platform
)
