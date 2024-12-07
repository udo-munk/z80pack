cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

add_subdirectory(../../lib ds3231)

project(read_temperature C CXX ASM)

pico_sdk_init()

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c)
pico_set_program_name(${PROJECT_NAME} ${PROJECT_NAME})
pico_set_program_version(${PROJECT_NAME} "0.1")

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

target_link_libraries(
    ${PROJECT_NAME}
    pico_stdlib
    hardware_i2c
    pico-ds3231
    )

pico_add_extra_outputs(${PROJECT_NAME})
