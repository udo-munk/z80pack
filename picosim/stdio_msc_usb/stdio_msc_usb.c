/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2024 Thomas Eberhardt
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tusb.h"
#include "stdio_msc_usb.h"

#include "pico/binary_info.h"
#include "pico/time.h"
#include "pico/stdio/driver.h"
#include "pico/mutex.h"
#include "hardware/irq.h"
#include "device/usbd_pvt.h" // for usbd_defer_func

static mutex_t stdio_msc_usb_mutex;

#if STDIO_MSC_USB_SUPPORT_CHARS_AVAILABLE_CALLBACK
static void (*chars_available_callback)(void*);
static void *chars_available_param;
#endif

// if this crit_sec is initialized, we are not in periodic timer mode, and must make sure
// we don't either create multiple one shot timers, or miss creating one. this crit_sec
// is used to protect the one_shot_timer_pending flag
static critical_section_t one_shot_timer_crit_sec;
static volatile bool one_shot_timer_pending;
#ifdef STDIO_MSC_USB_LOW_PRIORITY_IRQ
static_assert(STDIO_MSC_USB_LOW_PRIORITY_IRQ >= NUM_IRQS - NUM_USER_IRQS, "");
#define low_priority_irq_num STDIO_MSC_USB_LOW_PRIORITY_IRQ
#else
static uint8_t low_priority_irq_num;
#endif

static int64_t timer_task(__unused alarm_id_t id, __unused void *user_data) {
    int64_t repeat_time;
    if (critical_section_is_initialized(&one_shot_timer_crit_sec)) {
        critical_section_enter_blocking(&one_shot_timer_crit_sec);
        one_shot_timer_pending = false;
        critical_section_exit(&one_shot_timer_crit_sec);
        repeat_time = 0; // don't repeat
    } else {
        repeat_time = STDIO_MSC_USB_TASK_INTERVAL_US;
    }
    irq_set_pending(low_priority_irq_num);
    return repeat_time;
}

static void low_priority_worker_irq(void) {
    if (mutex_try_enter(&stdio_msc_usb_mutex, NULL)) {
        tud_task();
        mutex_exit(&stdio_msc_usb_mutex);
    } else {
        // if the mutex is already owned, then we are in non IRQ code in this file.
        //
        // it would seem simplest to just let that code call tud_task() at the end, however this
        // code might run during the call to tud_task() and we might miss a necessary tud_task() call
        //
        // if we are using a periodic timer (crit_sec is not initialized in this case),
        // then we are happy just to wait until the next tick, however when we are not using a periodic timer,
        // we must kick off a one-shot timer to make sure the tud_task() DOES run (this method
        // will be called again as a result, and will try the mutex_try_enter again, and if that fails
        // create another one shot timer again, and so on).
        if (critical_section_is_initialized(&one_shot_timer_crit_sec)) {
            bool need_timer;
            critical_section_enter_blocking(&one_shot_timer_crit_sec);
            need_timer = !one_shot_timer_pending;
            one_shot_timer_pending = true;
            critical_section_exit(&one_shot_timer_crit_sec);
            if (need_timer) {
                add_alarm_in_us(STDIO_MSC_USB_TASK_INTERVAL_US, timer_task, NULL, true);
            }
        }
    }
}

static void usb_irq(void) {
    irq_set_pending(low_priority_irq_num);
}

static void stdio_msc_usb_out_chars(const char *buf, int length) {
    static uint64_t last_avail_time;
    if (!mutex_try_enter_block_until(&stdio_msc_usb_mutex, make_timeout_time_ms(PICO_STDIO_DEADLOCK_TIMEOUT_MS))) {
        return;
    }
    if (stdio_msc_usb_connected()) {
        for (int i = 0; i < length;) {
            int n = length - i;
            int avail = (int) tud_cdc_write_available();
            if (n > avail) n = avail;
            if (n) {
                int n2 = (int) tud_cdc_write(buf + i, (uint32_t)n);
                tud_task();
                tud_cdc_write_flush();
                i += n2;
                last_avail_time = time_us_64();
            } else {
                tud_task();
                tud_cdc_write_flush();
                if (!stdio_msc_usb_connected() ||
                    (!tud_cdc_write_available() && time_us_64() > last_avail_time + STDIO_MSC_USB_STDOUT_TIMEOUT_US)) {
                    break;
                }
            }
        }
    } else {
        // reset our timeout
        last_avail_time = 0;
    }
    mutex_exit(&stdio_msc_usb_mutex);
}

int stdio_msc_usb_in_chars(char *buf, int length) {
    // note we perform this check outside the lock, to try and prevent possible deadlock conditions
    // with printf in IRQs (which we will escape through timeouts elsewhere, but that would be less graceful).
    //
    // these are just checks of state, so we can call them while not holding the lock.
    // they may be wrong, but only if we are in the middle of a tud_task call, in which case at worst
    // we will mistakenly think we have data available when we do not (we will check again), or
    // tud_task will complete running and we will check the right values the next time.
    //
    int rc = PICO_ERROR_NO_DATA;
    if (stdio_msc_usb_connected() && tud_cdc_available()) {
        if (!mutex_try_enter_block_until(&stdio_msc_usb_mutex, make_timeout_time_ms(PICO_STDIO_DEADLOCK_TIMEOUT_MS))) {
            return PICO_ERROR_NO_DATA; // would deadlock otherwise
        }
        if (stdio_msc_usb_connected() && tud_cdc_available()) {
            int count = (int) tud_cdc_read(buf, (uint32_t) length);
            rc = count ? count : PICO_ERROR_NO_DATA;
        } else {
            // because our mutex use may starve out the background task, run tud_task here (we own the mutex)
            tud_task();
        }
        mutex_exit(&stdio_msc_usb_mutex);
    }
    return rc;
}

#if STDIO_MSC_USB_SUPPORT_CHARS_AVAILABLE_CALLBACK
void tud_cdc_rx_cb(__unused uint8_t itf) {
    if (chars_available_callback) {
        usbd_defer_func(chars_available_callback, chars_available_param, false);
    }
}

void stdio_msc_usb_set_chars_available_callback(void (*fn)(void*), void *param) {
    chars_available_callback = fn;
    chars_available_param = param;
}
#endif

stdio_driver_t stdio_msc_usb = {
    .out_chars = stdio_msc_usb_out_chars,
    .in_chars = stdio_msc_usb_in_chars,
#if STDIO_MSC_USB_SUPPORT_CHARS_AVAILABLE_CALLBACK
    .set_chars_available_callback = stdio_msc_usb_set_chars_available_callback,
#endif
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
    .crlf_enabled = STDIO_MSC_USB_DEFAULT_CRLF
#endif

};

bool stdio_msc_usb_init(void) {
    if (get_core_num() != alarm_pool_core_num(alarm_pool_get_default())) {
        // included an assertion here rather than just returning false, as this is likely
        // a coding bug, rather than anything else.
        assert(false);
        return false;
    }
#if !PICO_NO_BI_STDIO_USB
    bi_decl_if_func_used(bi_program_feature("USB stdin / stdout"));
#endif

    mutex_init(&stdio_msc_usb_mutex);
    bool rc = true;
#ifdef STDIO_MSC_USB_LOW_PRIORITY_IRQ
    user_irq_claim(STDIO_MSC_USB_LOW_PRIORITY_IRQ);
#else
    low_priority_irq_num = (uint8_t) user_irq_claim_unused(true);
#endif
    irq_set_exclusive_handler(low_priority_irq_num, low_priority_worker_irq);
    irq_set_enabled(low_priority_irq_num, true);

    if (irq_has_shared_handler(USBCTRL_IRQ)) {
        // we can use a shared handler to notice when there may be work to do
        irq_add_shared_handler(USBCTRL_IRQ, usb_irq, PICO_SHARED_IRQ_HANDLER_LOWEST_ORDER_PRIORITY);
        critical_section_init_with_lock_num(&one_shot_timer_crit_sec, next_striped_spin_lock_num());
    } else {
        rc = add_alarm_in_us(STDIO_MSC_USB_TASK_INTERVAL_US, timer_task, NULL, true) >= 0;
        // we use initialization state of the one_shot_timer_critsec as a flag
        memset(&one_shot_timer_crit_sec, 0, sizeof(one_shot_timer_crit_sec));
    }
    if (rc) {
#if STDIO_MSC_USB_DISABLE_STDIO == 0
        stdio_set_driver_enabled(&stdio_msc_usb, true);
#endif
#if STDIO_MSC_USB_CONNECT_WAIT_TIMEOUT_MS
#if STDIO_MSC_USB_CONNECT_WAIT_TIMEOUT_MS > 0
        absolute_time_t until = make_timeout_time_ms(STDIO_MSC_USB_CONNECT_WAIT_TIMEOUT_MS);
#else
        absolute_time_t until = at_the_end_of_time;
#endif
        do {
            if (stdio_msc_usb_connected()) {
#if STDIO_MSC_USB_POST_CONNECT_WAIT_DELAY_MS != 0
                sleep_ms(STDIO_MSC_USB_POST_CONNECT_WAIT_DELAY_MS);
#endif
                break;
            }
            sleep_ms(10);
        } while (!time_reached(until));
#endif
    }
    return rc;
}

bool stdio_msc_usb_connected(void) {
#if STDIO_MSC_USB_CONNECTION_WITHOUT_DTR
    return tud_ready();
#else
    // this actually checks DTR
    return tud_cdc_connected();
#endif
}

bool stdio_msc_usb_disable_stdio(void) {
#if STDIO_MSC_USB_DISABLE_STDIO == 0
    if (!mutex_try_enter_block_until(&stdio_msc_usb_mutex, make_timeout_time_ms(PICO_STDIO_DEADLOCK_TIMEOUT_MS))) {
        return false;
    }
    stdio_set_driver_enabled(&stdio_msc_usb, false);
#endif
    return true;
}

void stdio_msc_usb_enable_stdio(void) {
#if STDIO_MSC_USB_DISABLE_STDIO == 0
    mutex_exit(&stdio_msc_usb_mutex);
    stdio_set_driver_enabled(&stdio_msc_usb, true);
#endif
}
