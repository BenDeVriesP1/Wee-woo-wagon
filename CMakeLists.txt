cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(wee_woo_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(wee_woo)


target_sources(wee_woo PRIVATE 
        wee_woo.c 
        Normal16KhzStereo.c)

# pico_enable_stdio_uart(wee_woo 1)
pico_enable_stdio_usb(wee_woo 1)


target_link_libraries(wee_woo PRIVATE
        pico_stdlib
        hardware_pio
        hardware_dma
        hardware_pio
        hardware_clocks
        )





pico_add_extra_outputs(wee_woo)