function(util_common targetName)
    pico_enable_stdio_usb(${targetName} 1)
    pico_enable_stdio_uart(${targetName} 0)
    # create map/bin/hex file etc.
    pico_add_extra_outputs(${targetName})

    target_include_directories(${targetName} PRIVATE .)
    target_include_directories(${targetName} PRIVATE ..)
    target_link_directories(${targetName} PRIVATE ..)
    target_link_libraries(${targetName} PRIVATE pi)
endfunction()

function(util_c targetName)
    add_executable(${targetName} ${targetName}.c ${ARGV})
    util_common(${targetName})
endfunction()

function(util_cpp targetName)
    add_executable(${targetName} ${targetName}.cpp ${ARGV})
    util_common(${targetName})
endfunction()

add_definitions(-DPI_PICO)

util_cpp(ble)
util_cpp(pico-audio pico-audio-wav.h)
util_cpp(servo)

add_custom_command(OUTPUT pico-audio-wav.h WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/utils COMMAND ../host-tools/wav-to-h pico-audio DEPENDS pico-audio.wav)
