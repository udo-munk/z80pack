cmake_minimum_required(VERSION 3.12)
# set(PICO_BOARD pico_w)
set (PICO_BOARD pico)
include(pico_sdk_import.cmake)

project(picosim C CXX ASM)
pico_sdk_init()

include_directories(
	.
	../z80core
)
add_executable(picosim
	picosim.c
	../z80core/sim1.c
	../z80core/sim2.c
	../z80core/sim3.c
	../z80core/sim4.c
	../z80core/sim5.c
	../z80core/sim6.c
	../z80core/sim7.c
	../z80core/simglb.c
	config.c
	memory.c
	iosim.c
)

target_link_libraries(picosim pico_stdlib)
pico_add_extra_outputs(picosim)

# enable uart output, enable usb output
pico_enable_stdio_uart(picosim 1)
pico_enable_stdio_usb(picosim 1)
