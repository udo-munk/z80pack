cmake_minimum_required(VERSION 3.13)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
set(CMAKE_BUILD_TYPE Release)
endif()

set(PICO_BOARD_HEADER_DIRS ${CMAKE_SOURCE_DIR})

# Set platform, board, and compiler
if(NOT PICO_PLATFORM)
	set(PICO_PLATFORM rp2040)
endif()
if(PICO_PLATFORM STREQUAL rp2040)
	set(PICO_BOARD pico)
	#set(PICO_BOARD pico_w)
	set(PICO_COMPILER pico_arm_cortex_m0plus_gcc)
elseif(PICO_PLATFORM STREQUAL rp2350-arm-s)
	set(PICO_BOARD pico2)
	set(PICO_COMPILER pico_arm_cortex_m33_gcc)
elseif(PICO_PLATFORM STREQUAL rp2350-riscv)
	set(PICO_BOARD pico2)
	set(PICO_COMPILER pico_riscv_gcc)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(picosim C CXX ASM)

pico_sdk_init()

set(Z80PACK ${CMAKE_SOURCE_DIR}/../../)

add_executable(${PROJECT_NAME}
	picosim.c
	disks.c
	simcfg.c
	simio.c
	simmem.c
	${Z80PACK}/iodevices/rtc80.c
	${Z80PACK}/iodevices/sd-fdc.c
	${Z80PACK}/z80core/sim8080.c
	${Z80PACK}/z80core/simcore.c
	${Z80PACK}/z80core/simdis.c
	${Z80PACK}/z80core/simglb.c
	${Z80PACK}/z80core/simice.c
	${Z80PACK}/z80core/simutil.c
	${Z80PACK}/z80core/simz80-cb.c
	${Z80PACK}/z80core/simz80-dd.c
	${Z80PACK}/z80core/simz80-ddcb.c
	${Z80PACK}/z80core/simz80-ed.c
	${Z80PACK}/z80core/simz80-fd.c
	${Z80PACK}/z80core/simz80-fdcb.c
	${Z80PACK}/z80core/simz80.c
)

# generate the header file into the source tree
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/WS2812.pio)

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}
	${Z80PACK}/iodevices
	${Z80PACK}/z80core
)

add_subdirectory(../libs/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src FatFs)
add_subdirectory(../libs/stdio_msc_usb stdio_msc_usb)
add_subdirectory(../libs/pico-ds3231/lib ds3231)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	PICO_STACK_SIZE=4096
	PICO_CORE1_STACK_SIZE=4096
	PICO_HEAP_SIZE=8192
	USBD_MANUFACTURER="Z80pack"
)
if(PICO_RP2040)
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		USBD_PRODUCT="Pi Pico RP2040"
		CONF_FILE="RP2040.DAT"
	)
else()
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		USBD_PRODUCT="Pi Pico RP2350"
		CONF_FILE="RP2350.DAT"
	)
endif()

# compiler diagnostic options
if(PICO_C_COMPILER_IS_GNU)
	target_compile_options(${PROJECT_NAME} PUBLIC -fdiagnostics-color=always)
elseif(PICO_C_COMPILER_IS_CLANG)
	target_compile_options(${PROJECT_NAME} PUBLIC -fcolor-diagnostics)
endif()
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra)
if(PICO_C_COMPILER_IS_CLANG)
	target_compile_options(${PROJECT_NAME} PUBLIC -Weverything)
endif()
# -Wno-unused-parameter because no-OS-FatFS-SD-SDIO-SPI-RPi-Pico isn't clean
target_compile_options(${PROJECT_NAME} PUBLIC -Wno-unused-parameter)

target_link_libraries(${PROJECT_NAME}
	hardware_i2c
	pico_stdlib
	tinyusb_device

	no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
	pico-ds3231
	stdio_msc_usb
)
if(PICO_BOARD STREQUAL pico_w)
target_link_libraries(${PROJECT_NAME}
	pico_cyw43_arch_none
)
endif()

pico_add_extra_outputs(${PROJECT_NAME})

target_link_options(${PROJECT_NAME} PRIVATE -Xlinker --print-memory-usage)

pico_set_program_name(${PROJECT_NAME} "z80pack picosim")
if(PICO_RP2040)
	pico_set_program_description(${PROJECT_NAME} "z80pack on Raspberry Pi Pico")
else()
	pico_set_program_description(${PROJECT_NAME} "z80pack on Raspberry Pi Pico 2")
endif()
pico_set_program_version(${PROJECT_NAME} "1.3")
pico_set_program_url(${PROJECT_NAME} "https://github.com/udo-munk/z80pack")

# enable UART in/out, enable USB in/out
pico_enable_stdio_uart(${PROJECT_NAME} 1)
#pico_enable_stdio_usb(${PROJECT_NAME} 1)
