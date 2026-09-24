#ifndef BW_PORT_H
#define BW_PORT_H
#include "simdefs.h"
uint64_t get_clock_us(void);
void sleep_for_us(unsigned long us);
void sleep_for_ms(unsigned ms);
#endif
