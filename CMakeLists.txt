cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/freertos/FreeRTOS-Kernel)
include(freertos/FreeRTOS_Kernel_import.cmake)

project(pico-freertos)
pico_sdk_init()
add_subdirectory(oled1_lib)

add_executable(pico-freertos
    main/main.c
)

target_include_directories(pico-freertos PRIVATE
    freertos/
)

target_link_libraries(pico-freertos PRIVATE
    oled1_lib
    pico_stdlib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    hardware_dma
    hardware_adc
    hardware_spi
)
target_compile_options(pico-freertos PRIVATE -Wall)
pico_add_extra_outputs(pico-freertos)