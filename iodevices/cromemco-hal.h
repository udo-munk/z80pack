/*
 * cromemco-hal.h
 *
 * Copyright (C) 2022 by David McNaughton
 *
 * Cromemco TU-ART hardware abstraction layer
 *
 * History:
 * 9-JUL-2022	1.0	Initial Release
 *
 */

#ifndef CROMEMCO_HAL_INC
#define CROMEMCO_HAL_INC

#include "sim.h"
#include "simdefs.h"

typedef enum tuart_port {
	TUART0A,
	TUART1A,
	TUART1B,
	MAX_TUART_PORT
} tuart_port_t;

typedef enum hal_dev {
	WEBTTYDEV,
	WEBTTY2DEV,
	WEBTTY3DEV,
	WEBPTRDEV,
	STDIODEV,
	SCKTSRV1DEV,
	SCKTSRV2DEV,
	MODEMDEV,
	NULLDEV,
	MAX_HAL_DEV
} hal_dev_t;

typedef struct hal_device {
	const char *name;
	bool	fallthrough;
	int	device_id;
	bool	(*alive)(int dev);
	void	(*status)(int dev, BYTE *stat);
	int	(*in)(int dev);
	void	(*out)(int dev, BYTE data);
} hal_device_t;

extern void hal_reset(void);

extern void hal_status_in(tuart_port_t dev, BYTE *stat);
extern int hal_data_in(tuart_port_t dev);
extern void hal_data_out(tuart_port_t dev, BYTE data);
extern bool hal_alive(tuart_port_t dev);

extern const char *tuart_port_name[MAX_TUART_PORT];
extern hal_device_t tuart[MAX_TUART_PORT][MAX_HAL_DEV];

#endif /* !CROMEMCO_HAL_INC */
