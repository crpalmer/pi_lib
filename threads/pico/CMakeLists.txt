include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
target_compile_options(FreeRTOS+FAT+CLI INTERFACE -Wno-error)

add_library(pi-threads-platform STATIC
  file.cpp
  freertos-heap.c
  sd-hw-config.c
  hooks.c
  pi-threads.cpp
  set-consoles-lock.cpp
)

target_include_directories(pi-threads-platform PUBLIC . )

target_link_libraries(pi-threads-platform PUBLIC
  pi-core
  pico_stdlib
  FreeRTOS+FAT+CLI
)
