if ("${PICO_BOARD}" STREQUAL pico OR "${PICO_BOARD}" STREQUAL pico_w OR "${PICO_BOARD}" STREQUAL pico2 OR "${PICO_BOARD}" STREQUAL pico2_w)
  include(${EXTERNAL_DIR}/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
else()
   message(FATAL_ERROR "Failed to identify the FreeRTOS kernel")
endif()

target_sources(lib-pi-threads INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/deep-sleep.cpp
  ${CMAKE_CURRENT_LIST_DIR}/file.cpp
  ${CMAKE_CURRENT_LIST_DIR}/freertos-heap.c
  ${CMAKE_CURRENT_LIST_DIR}/hooks.c
  ${CMAKE_CURRENT_LIST_DIR}/pi-threads-queue.cpp
  ${CMAKE_CURRENT_LIST_DIR}/pico-sd-cards.c
  ${CMAKE_CURRENT_LIST_DIR}/platform-threads.cpp
)

target_include_directories(lib-pi-threads INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(lib-pi-threads INTERFACE
    lib-pi
    FreeRTOS+FAT+CLI
)
