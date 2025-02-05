include(${EXTERNAL_DIR}/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

target_sources(lib-pi-threads INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/deep-sleep.cpp
  ${CMAKE_CURRENT_LIST_DIR}/file.cpp
  ${CMAKE_CURRENT_LIST_DIR}/freertos-heap.c
  ${CMAKE_CURRENT_LIST_DIR}/sd-hw-config.c
  ${CMAKE_CURRENT_LIST_DIR}/hooks.c
  ${CMAKE_CURRENT_LIST_DIR}/pi-threads.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pi-threads-queue.cpp
  ${CMAKE_CURRENT_LIST_DIR}/set-consoles-lock.cpp
)

target_include_directories(lib-pi-threads INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(lib-pi-threads INTERFACE
    lib-pi
    FreeRTOS+FAT+CLI
)
