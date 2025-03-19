/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024-2025 by Udo Munk & Thomas Eberhardt
 *
 * I/O simulation for picosim
 *
 * History:
 * 28-APR-2024 all I/O implemented for a first release
 * 09-MAY-2024 improved so that it can run some MITS Altair software
 * 11-MAY-2024 allow us to configure the port 255 value
 * 27-MAY-2024 moved io_in & io_out to simcore
 * 31-MAY-2024 added hardware control port for z80pack machines
 * 08-JUN-2024 implemented system reset
 * 09-JUN-2024 implemented boot ROM
 * 29-JUN-2024 implemented banked memory
 * 12-MAR-2025 added more memory banks for RP2350, 60 Hz timer, SIO2 & printer
 */

/* Raspberry SDK includes */
#include <stdio.h>
#if LIB_PICO_STDIO_USB || LIB_STDIO_MSC_USB
#include <tusb.h>
#endif
#include "pico/stdlib.h"
#include "hardware/uart.h"

/* Project includes */
#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"
#include "simcore.h"
#include "simio.h"

#include "rtc80.h"
#include "sd-fdc.h"
#include "rgbled.h"

#include "log.h"
static const char *TAG = "IO";

/*
 *	Forward declarations of the I/O functions
 *	for all port addresses.
 */
static BYTE sio1s_in(void), sio1d_in(void), sio2s_in(void), sio2d_in(void);
static BYTE prts_in(void), prtd_in(void), mmu_in(void), timer_in(void);
static BYTE hwctl_in(void), fpsw_in(void);
static void led_out(BYTE data), sio1d_out(BYTE data), sio2s_out(BYTE data);
static void sio2d_out(BYTE data), prts_out(BYTE data), prtd_out(BYTE data);
static void mmu_out(BYTE data), timer_out(BYTE data), hwctl_out(BYTE data);
static void fpsw_out(BYTE data), fpled_out(BYTE data);

static BYTE sio1_last;	/* last character received on SIO1 */
static BYTE sio2_last;	/* last character received on SIO2 */
       BYTE fp_value;	/* port 255 value, can be set from ICE or config() */
static bool timer;	/* 60 Hz timer enabled flag */
static BYTE hwctl_lock = 0xff; /* lock status hardware control port */

/*
 *	This array contains function pointers for every input
 *	I/O port (0 - 255), to do the required I/O.
 */
in_func_t *const port_in[256] = {
	[  0] = sio1s_in,	/* SIO1 status */
	[  1] = sio1d_in,	/* SIO1 read data */
	[  2] = sio2s_in,	/* SIO2 status */
	[  3] = sio2d_in,	/* SIO2 read data */
	[  4] = fdc_in,		/* FDC status */
	[  5] = prts_in,	/* printer status */
	[  6] = prtd_in,	/* printer read data */
	[ 64] = mmu_in,		/* MMU */
	[ 65] = clkc_in,	/* RTC read clock command */
	[ 66] = clkd_in,	/* RTC read clock data */
	[ 67] = timer_in,	/* 60 Hz timer status */
	[160] = hwctl_in,	/* virtual hardware control */
	[254] = fpsw_in,	/* mirror of port 255 */
	[255] = fpsw_in		/* read from front panel switches */
};

/*
 *	This array contains function pointers for every output
 *	I/O port (0 - 255), to do the required I/O.
 */
out_func_t *const port_out[256] = {
	[  0] = led_out,	/* RGB LED */
	[  1] = sio1d_out,	/* SIO1 write data */
	[  2] = sio2s_out,	/* SIO2 write status */
	[  3] = sio2d_out,	/* SIO2 write data */
	[  4] = fdc_out,	/* FDC command */
	[  5] = prts_out,	/* printer write status */
	[  6] = prtd_out,	/* printer write data */
	[ 64] = mmu_out,	/* MMU */
	[ 65] = clkc_out,	/* RTC write clock command */
	[ 66] = clkd_out,	/* RTC write clock data */
	[ 67] = timer_out,	/* 60 Hz timer control */
	[160] = hwctl_out,	/* virtual hardware control */
	[254] = fpsw_out,	/* write to front panel switches */
	[255] = fpled_out	/* write to front panel lights (dummy) */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 */
void exit_io(void)
{
	timer = false;		/* stop 60 Hz timer */
}

/*
 *	I/O handler for read SIO1 (Pico UART or USB Console CDC) status:
 *	bit 0 = 0, character available for input from tty
 *	bit 7 = 0, transmitter ready to write character to tty
 */
static BYTE sio1s_in(void)
{
	register BYTE stat = 0b10000001; /* initially not ready */

#if LIB_PICO_STDIO_UART
	uart_inst_t *my_uart = uart_default;

	if (uart_is_writable(my_uart))	/* check if output to UART is possible */
		stat &= 0b01111111;	/* if so flip status bit */
	if (uart_is_readable(my_uart))	/* check if there is input from UART */
		stat &= 0b11111110;	/* if so flip status bit */
#endif
#if LIB_PICO_STDIO_USB
	if (tud_cdc_connected()) {
		/* check if output to CDC is possible */
		if (tud_cdc_write_available())
			stat &= 0b01111111;	/* if so flip status bit */
		/* check if there is input from CDC */
		if (tud_cdc_available())
			stat &= 0b11111110;	/* if so flip status bit */
	}
#endif
#if LIB_STDIO_MSC_USB && !STDIO_MSC_USB_DISABLE_STDIO
	if (tud_cdc_n_connected(STDIO_MSC_USB_CONSOLE_ITF)) {
		/* check if output to CDC is possible */
		if (tud_cdc_n_write_available(STDIO_MSC_USB_CONSOLE_ITF))
			stat &= 0b01111111;	/* if so flip status bit */
		/* check if there is input from CDC */
		if (tud_cdc_n_available(STDIO_MSC_USB_CONSOLE_ITF))
			stat &= 0b11111110;	/* if so flip status bit */
	}
#endif

	return stat;
}

/*
 *	I/O handler for read SIO1 (Pico UART or USB Console CDC) data.
 */
static BYTE sio1d_in(void)
{
	bool input_avail = false;

#if LIB_PICO_STDIO_UART
	uart_inst_t *my_uart = uart_default;

	if (uart_is_readable(my_uart))
		input_avail = true;
#endif
#if LIB_PICO_STDIO_USB
	if (tud_cdc_connected() && tud_cdc_available())
		input_avail = true;
#endif
#if LIB_STDIO_MSC_USB && !STDIO_MSC_USB_DISABLE_STDIO
	if (tud_cdc_n_connected(STDIO_MSC_USB_CONSOLE_ITF) &&
	    tud_cdc_n_available(STDIO_MSC_USB_CONSOLE_ITF))
		input_avail = true;
#endif

	if (input_avail)
		sio1_last = getchar();

	return sio1_last;
}

/*
 *	I/O handler for read SIO2 (USB Console 2 CDC) status:
 *	bit 0 = 0, character available for input from tty
 *	bit 7 = 0, transmitter ready to write character to tty
 */
static BYTE sio2s_in(void)
{
	register BYTE stat = 0b10000001; /* initially not ready */

#if LIB_STDIO_MSC_USB
	if (tud_cdc_n_connected(STDIO_MSC_USB_CONSOLE2_ITF)) {
		/* check if output to CDC is possible */
		if (tud_cdc_n_write_available(STDIO_MSC_USB_CONSOLE2_ITF))
			stat &= 0b01111111;	/* if so flip status bit */
		/* check if there is input from CDC */
		if (tud_cdc_n_available(STDIO_MSC_USB_CONSOLE2_ITF))
			stat &= 0b11111110;	/* if so flip status bit */
	}
#endif

	return stat;
}

/*
 *	I/O handler for read SIO2 (USB Console 2 CDC) data.
 */
static BYTE sio2d_in(void)
{
#if LIB_STDIO_MSC_USB
	if (tud_cdc_n_connected(STDIO_MSC_USB_CONSOLE2_ITF) &&
	    tud_cdc_n_available(STDIO_MSC_USB_CONSOLE2_ITF))
		sio2_last = tud_cdc_n_read_char(STDIO_MSC_USB_CONSOLE2_ITF);
#endif

	return sio2_last;
}

/*
 *	I/O handler for read printer (USB Printer CDC) status:
 */
static BYTE prts_in(void)
{
	register BYTE stat = 0; /* initially not ready */

#if LIB_STDIO_MSC_USB
	if (tud_cdc_n_connected(STDIO_MSC_USB_PRINTER_ITF) &&
	    tud_cdc_n_write_available(STDIO_MSC_USB_PRINTER_ITF))
		stat = 0xff;
#endif

	return stat;
}

/*
 *	I/O handler for read printer (USB Printer CDC) data:
 *	always read an EOF from the printer
 */
static BYTE prtd_in(void)
{
	return 0x1a;	/* CP/M EOF */
}

/*
 *	read MMU register
 *	returns maximum bank in upper nibble
 *	and currently selected bank in lower nibble
 */
static BYTE mmu_in(void)
{
	return (NUMSEG << 4) | selbnk;
}

/*
 *	read 60 Hz timer enabled status
 */
static BYTE timer_in(void)
{
	return timer ? 1 : 0;
}

/*
 *	Input from virtual hardware control port
 *	returns lock status of the port
 */
static BYTE hwctl_in(void)
{
	return hwctl_lock;
}

/*
 *	Read virtual front panel switches state
 */
static BYTE fpsw_in(void)
{
	return fp_value;
}

/*
 *	Switch RGB LED blue on/off.
 */
static void led_out(BYTE data)
{
	if (!data) {
		/* 0 switches LED off */
		put_pixel(0x000000); /* LED off */
		sleep_us(300);
	} else {
		/* everything else on */
		put_pixel(0x404000); /* LED on */
		sleep_us(300);
	}
}

/*
 *	Write byte to Pico UART or USB Console CDC.
 */
static void sio1d_out(BYTE data)
{
	putchar_raw((int) data & 0x7f); /* strip parity, some software won't */
}

/*
 *	Write SIO2 status (no function).
 */
static void sio2s_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	Write SIO2 (USB Console 2 CDC) data.
 */
static void sio2d_out(BYTE data)
{
#if LIB_STDIO_MSC_USB
	if (tud_cdc_n_connected(STDIO_MSC_USB_CONSOLE2_ITF)) {
		if (!tud_cdc_n_write_available(STDIO_MSC_USB_CONSOLE2_ITF))
			tud_cdc_n_write_flush(STDIO_MSC_USB_CONSOLE2_ITF);
		tud_cdc_n_write_char(STDIO_MSC_USB_CONSOLE2_ITF, data);
		tud_cdc_n_write_flush(STDIO_MSC_USB_CONSOLE2_ITF);
	}
#else
	UNUSED(data);
#endif
}

/*
 *	Write printer status (no function).
 */
static void prts_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	Write printer (USB Printer CDC) data.
 */
static void prtd_out(BYTE data)
{
#if LIB_STDIO_MSC_USB
	if ((data != '\r') && (data != 0x00) &&
	    tud_cdc_n_connected(STDIO_MSC_USB_PRINTER_ITF)) {
		if (!tud_cdc_n_write_available(STDIO_MSC_USB_PRINTER_ITF))
			tud_cdc_n_write_flush(STDIO_MSC_USB_PRINTER_ITF);
		tud_cdc_n_write_char(STDIO_MSC_USB_PRINTER_ITF, data);
		tud_cdc_n_write_flush(STDIO_MSC_USB_PRINTER_ITF);
	}
#else
	UNUSED(data);
#endif
}

/*
 *	write MMU register
 */
static void mmu_out(BYTE data)
{
	if (selbnk > NUMSEG) {
		LOGE(TAG, "%04x: trying to select non-existing bank %d",
		     PC, data);
		cpu_error = IOERROR;
		cpu_state = ST_STOPPED;
		return;
	}
	selbnk = data;
	if (selbnk != 0)
		curbnk = bnks[selbnk - 1];
}

/*
 *	timer interrupt causes maskable CPU interrupt
 */
static int64_t timer_alarm(alarm_id_t id, void *user_data)
{
	UNUSED(id);
	UNUSED(user_data);

	if (timer) {
		int_data = 0xff;	/* RST 38H for IM 0, 0FFH for IM 2 */
		int_int = true;
		return -16667L;		/* reschedule alarm */
	} else
		return 0L;		/* do not reschedule alarm */
}

/*
 *	write 60 Hz timer (start or stop)
 */
static void timer_out(BYTE data)
{
	if (data == 1) {
		if (!timer) {
			timer = true;
			add_alarm_in_us(16667L, timer_alarm, NULL, true);
		}
	} else
		timer = false;
}

/*
 *	Port is locked until magic number 0xaa is received!
 *
 *	Virtual hardware control output.
 *	Used to shutdown and switch CPU's.
 *
 *	bit 4 = 1	switch CPU model to 8080
 *	bit 5 = 1	switch CPU model to Z80
 *	bit 6 = 1	reset system
 *	bit 7 = 1	halt emulation via I/O
 */
static void hwctl_out(BYTE data)
{
	/* if port is locked do nothing */
	if (hwctl_lock && (data != 0xaa))
		return;

	/* unlock port ? */
	if (hwctl_lock && (data == 0xaa)) {
		hwctl_lock = 0;
		return;
	}

	/* process output to unlocked port */
	/* but first lock port again */
	hwctl_lock = 0xff;

	if (data & 128) {
		cpu_error = IOHALT;
		cpu_state = ST_STOPPED;
		return;
	}

	if (data & 64) {
		timer = false;		/* stop 60 Hz timer */
		reset_cpu();		/* reset CPU */
		reset_memory();		/* reset memory */
		PC = 0xff00;		/* power on jump to boot ROM */
		return;
	}

#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
	if (data & 32) {		/* switch cpu model to Z80 */
		switch_cpu(Z80);
		return;
	}

	if (data & 16) {		/* switch cpu model to 8080 */
		switch_cpu(I8080);
		return;
	}
#endif
}

/*
 *	This allows to set the virtual front panel switches with ICE p command
 */
static void fpsw_out(BYTE data)
{
	fp_value = data;
}

/*
 *	Write output to front panel lights (dummy)
 */
static void fpled_out(BYTE data)
{
	UNUSED(data);
}
