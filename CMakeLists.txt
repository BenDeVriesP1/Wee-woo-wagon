cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(wee_woo)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(wee_woo wee_woo.c)

pico_generate_pio_header(wee_woo ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.pio)

target_sources(wee_woo PRIVATE 
        Fast16KhzMono.c
        BetterLong16KhzMono.c
        EEEOOOO16monokhz.c
        Normal16KhzMono.c
        debug.c
        i2s_audio.c)

# pico_enable_stdio_uart(wee_woo 1)
pico_enable_stdio_usb(wee_woo 1)


target_link_libraries(wee_woo PRIVATE
        pico_stdlib
        hardware_i2c
        hardware_pio
        hardware_dma
        hardware_clocks
        )





pico_add_extra_outputs(wee_woo)