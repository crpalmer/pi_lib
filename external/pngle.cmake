add_library(pngle INTERFACE)
target_include_directories(pngle INTERFACE ${CMAKE_CURRENT_LIST_DIR}/pngle/src)
target_sources(pngle INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/pngle/src/pngle.c
  ${CMAKE_CURRENT_LIST_DIR}/pngle/src/miniz.c
)
