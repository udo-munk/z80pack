/* SPDX-License-Identifier: BSD-3-Clause */
#include "simport.h"
#ifdef _WIN32
#include <windows.h>
uint64_t get_clock_us(void)
{
    LARGE_INTEGER now, rate;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&rate);
    return (uint64_t)(now.QuadPart / rate.QuadPart) * 1000000 +
           (uint64_t)(now.QuadPart % rate.QuadPart) * 1000000 / rate.QuadPart;
}
void sleep_for_us(unsigned long us) { Sleep((us + 999) / 1000); }
#else
#include <time.h>
#include <errno.h>
uint64_t get_clock_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}
void sleep_for_us(unsigned long us)
{
    struct timespec ts = { us / 1000000, (us % 1000000) * 1000 };
    while (nanosleep(&ts, &ts) && errno == EINTR) {}
}
#endif
void sleep_for_ms(unsigned ms) { sleep_for_us((unsigned long)ms * 1000); }
