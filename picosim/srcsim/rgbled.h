#ifndef RGBLED_INC
#define RGBLED_INC

#include "hardware/pio.h"
#include "WS2812.pio.h"

extern uint sm;
extern PIO pio;

static inline void put_pixel(uint32_t pixel_grb) {
	pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

#endif /* !RGBLED_INC */
