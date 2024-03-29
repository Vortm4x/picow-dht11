cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)
include(pico_sdk_import.cmake)

project(pico-dht11-driver C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(lwip_import.cmake)

pico_sdk_init()


add_executable(${PROJECT_NAME}
    src/main.c
    src/dht11_driver.h src/dht11_driver.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include/
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_stdlib
    pico_cyw43_arch_none
    LWIP_PORT
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

# Run the entire project in SRAM
# pico_set_binary_type(pico-freertos copy_to_ram)

pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)