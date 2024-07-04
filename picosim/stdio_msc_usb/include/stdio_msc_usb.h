/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2024 Thomas Eberhardt
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _STDIO_MSC_USB_H
#define _STDIO_MSC_USB_H

#include "pico/stdio.h"

/** \brief Support for stdin/stdout over USB serial (CDC)
 *  \defgroup stdio_msc_usb stdio_msc_usb
 *  \ingroup pico_stdio
 *
 *  Linking this library in the CMake will add USB CDC to the drivers used for standard input/output
 *
 *  It takes control of a lower level IRQ and sets up a periodic background task.
 *
 *  This library also includes (by default) functionality to enable the RP2040 to be reset over the USB interface.
 */

// PICO_CONFIG: STDIO_MSC_USB_DEFAULT_CRLF, Default state of CR/LF translation for USB output, type=bool, default=PICO_STDIO_DEFAULT_CRLF, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_DEFAULT_CRLF
#define STDIO_MSC_USB_DEFAULT_CRLF PICO_STDIO_DEFAULT_CRLF
#endif

// PICO_CONFIG: STDIO_MSC_USB_STDOUT_TIMEOUT_US, Number of microseconds to be blocked trying to write USB output before assuming the host has disappeared and discarding data, default=500000, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_STDOUT_TIMEOUT_US
#define STDIO_MSC_USB_STDOUT_TIMEOUT_US 500000
#endif

// todo perhaps unnecessarily frequent?
// PICO_CONFIG: STDIO_MSC_USB_TASK_INTERVAL_US, Period of microseconds between calling tud_task in the background, default=1000, advanced=true, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_TASK_INTERVAL_US
#define STDIO_MSC_USB_TASK_INTERVAL_US 1000
#endif

// PICO_CONFIG: STDIO_MSC_USB_LOW_PRIORITY_IRQ, Explicit User IRQ number to claim for tud_task() background execution instead of letting the implementation pick a free one dynamically (deprecated), advanced=true, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_LOW_PRIORITY_IRQ
// this variable is no longer set by default (one is claimed dynamically), but will be respected if specified
#endif

// PICO_CONFIG: STDIO_MSC_USB_ENABLE_RESET_VIA_BAUD_RATE, Enable/disable resetting into BOOTSEL mode if the host sets the baud rate to a magic value (STDIO_MSC_USB_RESET_MAGIC_BAUD_RATE), type=bool, default=1, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_ENABLE_RESET_VIA_BAUD_RATE
#define STDIO_MSC_USB_ENABLE_RESET_VIA_BAUD_RATE 1
#endif

// PICO_CONFIG: STDIO_MSC_USB_RESET_MAGIC_BAUD_RATE, baud rate that if selected causes a reset into BOOTSEL mode (if STDIO_MSC_USB_ENABLE_RESET_VIA_BAUD_RATE is set), default=1200, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_RESET_MAGIC_BAUD_RATE
#define STDIO_MSC_USB_RESET_MAGIC_BAUD_RATE 1200
#endif

// PICO_CONFIG: STDIO_MSC_USB_CONNECT_WAIT_TIMEOUT_MS, Maximum number of milliseconds to wait during initialization for a CDC connection from the host (negative means indefinite) during initialization, default=0, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_CONNECT_WAIT_TIMEOUT_MS
#define STDIO_MSC_USB_CONNECT_WAIT_TIMEOUT_MS 0
#endif

// PICO_CONFIG: STDIO_MSC_USB_POST_CONNECT_WAIT_DELAY_MS, Number of extra milliseconds to wait when using STDIO_MSC_USB_CONNECT_WAIT_TIMEOUT_MS after a host CDC connection is detected (some host terminals seem to sometimes lose transmissions sent right after connection), default=50, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_POST_CONNECT_WAIT_DELAY_MS
#define STDIO_MSC_USB_POST_CONNECT_WAIT_DELAY_MS 50
#endif

// PICO_CONFIG: STDIO_MSC_USB_RESET_BOOTSEL_ACTIVITY_LED, Optionally define a pin to use as bootloader activity LED when BOOTSEL mode is entered via USB (either VIA_BAUD_RATE or VIA_VENDOR_INTERFACE), type=int, min=0, max=29, group=stdio_msc_usb

// PICO_CONFIG: STDIO_MSC_USB_RESET_BOOTSEL_FIXED_ACTIVITY_LED, Whether the pin specified by STDIO_MSC_USB_RESET_BOOTSEL_ACTIVITY_LED is fixed or can be modified by picotool over the VENDOR USB interface, type=bool, default=0, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_RESET_BOOTSEL_FIXED_ACTIVITY_LED
#define STDIO_MSC_USB_RESET_BOOTSEL_FIXED_ACTIVITY_LED 0
#endif

// Any modes disabled here can't be re-enabled by picotool via VENDOR_INTERFACE.
// PICO_CONFIG: STDIO_MSC_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK, Optionally disable either the mass storage interface (bit 0) or the PICOBOOT interface (bit 1) when entering BOOTSEL mode via USB (either VIA_BAUD_RATE or VIA_VENDOR_INTERFACE), type=int, min=0, max=3, default=0, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK
#define STDIO_MSC_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK 0u
#endif

// PICO_CONFIG: STDIO_MSC_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE, Enable/disable resetting into BOOTSEL mode via an additional VENDOR USB interface - enables picotool based reset, type=bool, default=1, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE
#define STDIO_MSC_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE 1
#endif

// PICO_CONFIG: STDIO_MSC_USB_RESET_INTERFACE_SUPPORT_RESET_TO_BOOTSEL, If vendor reset interface is included allow rebooting to BOOTSEL mode, type=bool, default=1, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_RESET_INTERFACE_SUPPORT_RESET_TO_BOOTSEL
#define STDIO_MSC_USB_RESET_INTERFACE_SUPPORT_RESET_TO_BOOTSEL 1
#endif

// PICO_CONFIG: STDIO_MSC_USB_RESET_INTERFACE_SUPPORT_RESET_TO_FLASH_BOOT, If vendor reset interface is included allow rebooting with regular flash boot, type=bool, default=1, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_RESET_INTERFACE_SUPPORT_RESET_TO_FLASH_BOOT
#define STDIO_MSC_USB_RESET_INTERFACE_SUPPORT_RESET_TO_FLASH_BOOT 1
#endif

// PICO_CONFIG: STDIO_MSC_USB_RESET_RESET_TO_FLASH_DELAY_MS, delays in ms before rebooting via regular flash boot, default=100, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_RESET_RESET_TO_FLASH_DELAY_MS
#define STDIO_MSC_USB_RESET_RESET_TO_FLASH_DELAY_MS 100
#endif

// PICO_CONFIG: STDIO_MSC_USB_DISABLE_STDIO, Disable stdio support, type=bool, default=0, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_DISABLE_STDIO
#define STDIO_MSC_USB_DISABLE_STDIO 0
#endif
#if STDIO_MSC_USB_DISABLE_STDIO
#define STDIO_MSC_USB_CONNECTION_WITHOUT_DTR 1
#endif

// PICO_CONFIG: STDIO_MSC_USB_CONNECTION_WITHOUT_DTR, Disable use of DTR for connection checking meaning connection is assumed to be valid, type=bool, default=0, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_CONNECTION_WITHOUT_DTR
#define STDIO_MSC_USB_CONNECTION_WITHOUT_DTR 0
#endif

// PICO_CONFIG: STDIO_MSC_USB_DEVICE_SELF_POWERED, Set USB device as self powered device, type=bool, default=0, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_DEVICE_SELF_POWERED
#define STDIO_MSC_USB_DEVICE_SELF_POWERED 0
#endif

// PICO_CONFIG: STDIO_MSC_USB_SUPPORT_CHARS_AVAILABLE_CALLBACK, Enable USB STDIO support for stdio_set_chars_available_callback. Can be disabled to make use of USB CDC RX callback elsewhere, type=bool, default=1, group=stdio_msc_usb
#ifndef STDIO_MSC_USB_SUPPORT_CHARS_AVAILABLE_CALLBACK
#define STDIO_MSC_USB_SUPPORT_CHARS_AVAILABLE_CALLBACK 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern stdio_driver_t stdio_msc_usb;

/*! \brief Explicitly initialize USB stdio and add it to the current set of stdin drivers
 *  \ingroup stdio_msc_usb
 *
 *  \ref STDIO_MSC_USB_CONNECT_WAIT_TIMEOUT_MS can be set to cause this method to wait for a CDC connection
 *  from the host before returning, which is useful if you don't want any initial stdout output to be discarded
 *  before the connection is established.
 *
 *  \return true if the USB CDC was initialized, false if an error occurred
 */
bool stdio_msc_usb_init(void);

/*! \brief Check if there is an active stdio CDC connection to a host
 *  \ingroup stdio_msc_usb
 *
 *  \return true if stdio is connected over CDC
 */
bool stdio_msc_usb_connected(void);

void stdio_msc_usb_enable_irq_tud_task(void);

void stdio_msc_usb_disable_irq_tud_task(void);

void stdio_msc_usb_do_msc(void);

#ifdef __cplusplus
}
#endif

#endif
