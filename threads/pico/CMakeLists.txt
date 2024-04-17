include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

add_library(pi-threads-platform STATIC
  hooks.c
  pi-threads.c
)

target_link_libraries(pi-threads-platform PUBLIC
  pi-core
  pico_stdlib
  FreeRTOS-Kernel-Heap4
)
