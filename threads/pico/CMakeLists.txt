include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

add_library(pi-threads-platform STATIC
  file.cpp
  sd-hw-config.c
  hooks.c
  pi-threads.cpp
  set-consoles-lock.cpp
)

target_include_directories(pi-threads-platform PUBLIC . )

target_link_libraries(pi-threads-platform PUBLIC
  pi-core
  pico_stdlib
  FreeRTOS-Kernel-Heap4
  FreeRTOS+FAT+CLI
)
