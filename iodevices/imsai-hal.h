/*
 * imsai-hal.c
 *
 * Copyright (C) 2021 by David McNaiughton
 *
 * IMSAI SIO-2 hardware abstraction layer
 *
 * History:
 * 1-JUL-2021    1.0     Initial Release 
 *
 */

enum sio_port {
	SIO1A,
	SIO1B,
	SIO2A,
	SIO2B,
    MAX_SIO_PORT
};

typedef enum sio_port sio_port_t;

enum hal_dev {
    WEBTTYDEV,
    WEBPTRDEV,
    STDIODEV,
    SCKTSRVDEV,
    MODEMDEV,
    VIOKBD,
    NULLDEV,
    MAX_HAL_DEV
};

struct hal_device {
    char *name;
    int fallthrough;
	int	(*alive)(void);
	void (*status)(BYTE *stat);
	int (*in)(void);
	void (*out)(BYTE);
    int (*cd)(void);
};

typedef struct hal_device hal_device_t;

extern void hal_reset(void);

extern void hal_status_in(sio_port_t sio, BYTE *stat);
extern int hal_data_in(sio_port_t sio);
extern void hal_data_out(sio_port_t sio, BYTE data);
extern int hal_carrier_detect(sio_port_t sio);

extern const char *sio_port_name[MAX_SIO_PORT];
extern hal_device_t sio[MAX_SIO_PORT][MAX_HAL_DEV];
