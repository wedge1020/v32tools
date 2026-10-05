cmake_minimum_required(VERSION 3.13)

# Board: Waveshare RP2350-USB-A (USB-A host port on GPIO 12/13 via PIO)
set(PICO_BOARD waveshare_rp2350_usb_a CACHE STRING "Board type")

# Pico-PIO-USB is needed for the USB-A host port. Either pass
# -DPICO_PIO_USB_PATH=<path> or clone it next to this file
if(NOT DEFINED PICO_PIO_USB_PATH)
    set(PICO_PIO_USB_PATH ${CMAKE_CURRENT_LIST_DIR}/Pico-PIO-USB)
endif()

# Pico SDK location is taken from PICO_SDK_PATH
if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(v32kbd_pico C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()

if(NOT TARGET tinyusb_pico_pio_usb)
    message(FATAL_ERROR "Pico-PIO-USB not found: set PICO_PIO_USB_PATH")
endif()

add_executable(v32kbd_pico
    src/main.c
    src/keymap.c
    src/led.c
    src/usb_descriptors.c
)

# status light: WS2812 driven with PIO
pico_generate_pio_header(v32kbd_pico ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

target_include_directories(v32kbd_pico PRIVATE src)

target_link_libraries(v32kbd_pico PRIVATE
    pico_stdlib
    hardware_pio
    tinyusb_device
    tinyusb_host
    tinyusb_pico_pio_usb
)

# no stdio: both USB ports are in use by the adapter itself
pico_enable_stdio_usb(v32kbd_pico 0)
pico_enable_stdio_uart(v32kbd_pico 0)

# generates v32kbd_pico.uf2
pico_add_extra_outputs(v32kbd_pico)
