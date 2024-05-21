cmake_minimum_required(VERSION 3.12)
#set (PICO_BOARD pico)
set(PICO_BOARD pico_w)
include(pico_sdk_import.cmake)

project(picosim C CXX ASM)
pico_sdk_init()

include_directories(
	.
	../z80core
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
	config.c
	memsim.c
	iosim.c
)

# for Pico
#target_link_libraries(picosim pico_stdlib)
# for Pico W
target_link_libraries(picosim pico_stdlib pico_cyw43_arch_none)

pico_add_extra_outputs(picosim)

# enable UART output, disable USB output
pico_enable_stdio_uart(picosim 1)
pico_enable_stdio_usb(picosim 0)
