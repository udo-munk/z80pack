cmake_minimum_required(VERSION 3.12)

# Set platform, board and compiler
set(PICO_PLATFORM rp2040)
set (PICO_BOARD pico)
#set(PICO_BOARD pico_w)
set(PICO_COMPILER pico_arm_gcc)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(picosim C CXX ASM)
pico_sdk_init()

include_directories(
	.
	../z80core
	../iodevices
)

add_executable(picosim
	picosim.c
	../z80core/simcore.c
	../z80core/simglb.c
	../z80core/simdis.c
	../z80core/simice.c
	../z80core/sim8080.c
	../z80core/simz80.c
	../z80core/simz80-cb.c
	../z80core/simz80-dd.c
	../z80core/simz80-ddcb.c
	../z80core/simz80-ed.c
	../z80core/simz80-fd.c
	../z80core/simz80-fdcb.c
	../iodevices/sd-fdc.c
	simcfg.c
	simmem.c
	simio.c
)

add_subdirectory(
	no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src FatFs
)

target_compile_definitions(picosim PRIVATE
	PICO_STACK_SIZE=4096
	PICO_CORE1_STACK_SIZE=4096
	PICO_HEAP_SIZE=8192
)

target_compile_definitions(picosim INTERFACE
	PICO_DEFAULT_UART=0
)

# for Pico
target_link_libraries(picosim
	pico_stdlib
	no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
)
# for Pico W
#target_link_libraries(picosim
#	pico_stdlib
#	pico_cyw43_arch_none
#	no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
#)

pico_add_extra_outputs(picosim)

set_property(TARGET picosim APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")

pico_set_program_name(picosim "z80pack picosim")
pico_set_program_description(picosim "z80pack on Raspberry Pi Pico")
pico_set_program_version(picosim "1.2")
pico_set_program_url(picosim "https://github.com/udo-munk/z80pack")

# disable UART in/out, enable USB in/out
pico_enable_stdio_uart(picosim 0)
pico_enable_stdio_usb(picosim 1)
