set(EXTERNAL_DIR "${CMAKE_CURRENT_LIST_DIR}/external")

add_subdirectory(external)

add_subdirectory(core)
add_subdirectory(threads)
add_subdirectory(audio)
add_subdirectory(net)
if(DEFINED BUILD_EXAMPLES)
  message("Building example: ${BUILD_EXAMPLES}")
  add_subdirectory(examples)
endif()
