 /*
 * imsai-hal.c
 *
 * Copyright (C) 2021 by David McNaiughton
 *
 * IMSAI SIO-2 hardware abstraction layer
 *
 * History:
 * 1-JUL-2021	1.0	Initial Release
 *
 */

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simio.h"

#include "unix_terminal.h"
#include "unix_network.h"
#include "imsai-vio.h"
#ifdef HAS_NETSERVER
#include "netsrv.h"
#endif
#include "imsai-hal.h"

/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#define LOG_LOCAL_LEVEL LOG_WARN
#include "log.h"
static const char *TAG = "HAL";

/* -------------------- NULL device HAL -------------------- */

static int null_alive(void) {
    return 1; /* NULL is always alive */
}
#if !defined(HAS_NETSERVER) || !defined(HAS_MODEM)
static int null_dead(void) {
    return 0; /* NULL is always dead */
}
#endif
static void null_status(BYTE *stat) {
    UNUSED(stat);

    return;
}
static int null_in(void) {
    return -1;
}
static void null_out(BYTE data) {
    UNUSED(data);

    return;
}
static int null_cd(void) {
    return 0;
}

/* -------------------- VIOKBD HAL -------------------- */

static int vio_kbd_alive(void) {
#ifdef HAS_NETSERVER
    if (n_flag) {
        /* VIO (webUI) keyboard is only alive if websocket is connected */
        return net_device_alive(DEV_VIO);
    } else {
#endif
        /* VIO (xterm) keyboard is always alive */
        return 1;
#ifdef HAS_NETSERVER
    }
#endif

}
static void vio_kbd_status(BYTE *stat)
{
	*stat = imsai_kbd_status;
}
static int vio_kbd_in(void)
{
	int data;

	if (imsai_kbd_data == -1)
		return -1;

	/* take over data and reset */
	data = imsai_kbd_data;
	imsai_kbd_data = -1;
	imsai_kbd_status = 0;

	return data;
}
static void vio_kbd_out(BYTE data) {
    UNUSED(data);
}

/* -------------------- WEBTTY HAL -------------------- */

#ifdef HAS_NETSERVER
static int net_tty_alive(void) {
    if (n_flag) {
        /* WEBTTY is only alive if websocket is connected */
        return net_device_alive(DEV_TTY);
    } else {
        return 0;
    }
}
static void net_tty_status(BYTE *stat) {
    *stat &= (BYTE)(~3);
    if (n_flag) {
        if (net_device_poll(DEV_TTY)) {
            *stat |= 2;
        }
        *stat |= 1;
    }
}
static int net_tty_in(void) {
    if (n_flag) {
        return net_device_get(DEV_TTY);
    } else {
        return -1;
    }
}
static void net_tty_out(BYTE data) {
    if (n_flag) {
        net_device_send(DEV_TTY, (char *)&data, 1);
    }
}
#endif

/* -------------------- WEBPTR HAL -------------------- */

#ifdef HAS_NETSERVER
static int net_ptr_alive(void) {
    if (n_flag) {
        /* WEBPTR is only alive if websocket is connected */
        return net_device_alive(DEV_PTR);
    } else {
        return 0;
    }
}
static void net_ptr_status(BYTE *stat) {
    *stat &= (BYTE)(~3);
    if (n_flag) {
        if (net_device_poll(DEV_PTR)) {
            *stat |= 2;
        }
        *stat |= 1;
    }
}
static int net_ptr_in(void) {
    if (n_flag) {
        return net_device_get(DEV_PTR);
    } else {
        return -1;
    }
}
static void net_ptr_out(BYTE data) {
    if (n_flag) {
        net_device_send(DEV_PTR, (char *)&data, 1);
    }
}
#endif

/* -------------------- STDIO HAL -------------------- */

static int stdio_alive(void) {
    return 1; /* STDIO is always alive */
}
static void stdio_status(BYTE *stat) {
    struct pollfd p[1];

    p[0].fd = fileno(stdin);
    p[0].events = POLLIN;
    p[0].revents = 0;
    poll(p, 1, 0);
    *stat &= (BYTE)(~3);
    if (p[0].revents & POLLIN)
        *stat |= 2;
    if (p[0].revents & POLLNVAL) {
        LOGE(TAG, "can't use terminal, try 'screen simulation ...'");
        cpu_error = IOERROR;
        cpu_state = ST_STOPPED;
    }
    *stat |= 1;

}
static int stdio_in(void) {
    int data;
	struct pollfd p[1];

again:
    /* if no input waiting return last */
    p[0].fd = fileno(stdin);
    p[0].events = POLLIN;
    p[0].revents = 0;
    poll(p, 1, 0);
    if (!(p[0].revents & POLLIN))
        return -1;

    if (read(fileno(stdin), &data, 1) == 0) {
        /* try to reopen tty, input redirection exhausted */
        if (freopen("/dev/tty", "r", stdin) == NULL)
            LOGE(TAG, "can't reopen /dev/tty");
        set_unix_terminal();
        goto again;
    }

    return data;
}
static void stdio_out(BYTE data) {

again:
    if (write(fileno(stdout), (char *) &data, 1) != 1) {
        if (errno == EINTR) {
            goto again;
        } else {
            LOGE(TAG, "can't write data");
            cpu_error = IOERROR;
            cpu_state = ST_STOPPED;
        }
    }
}

/* -------------------- SOCKET SERVER HAL -------------------- */

static int scktsrv_alive(void) {

    struct pollfd p[1];

	/* if socket not connected check for a new connection */
	if (ucons[0].ssc == 0) {
		p[0].fd = ucons[0].ss;
		p[0].events = POLLIN;
		p[0].revents = 0;
		poll(p, 1, 0);
		/* accept a new connection */
		if (p[0].revents) {
			if ((ucons[0].ssc = accept(ucons[0].ss, NULL,
			     NULL)) == -1) {
				LOGW(TAG, "can't accept server socket");
				ucons[0].ssc = 0;
			}
		}
	}

	return ucons[0].ssc; /* SCKTSRV is alive if there is an open socket */
}
static void scktsrv_status(BYTE *stat) {

    struct pollfd p[1];

	/* if socket is connected check for I/O */
	if (ucons[0].ssc != 0) {
		p[0].fd = ucons[0].ssc;
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
        *stat &= (BYTE)(~3);
		if (p[0].revents & POLLIN)
			*stat |= 2;
		if (p[0].revents & POLLOUT)
			*stat |= 1;
	} else {
		*stat = 0;
	}
}
static int scktsrv_in(void) {
    BYTE data;
	struct pollfd p[1];

	/* if not connected return last */
	if (ucons[0].ssc == 0)
		return -1;

	/* if no input waiting return last */
	p[0].fd = ucons[0].ssc;
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return -1;

	if (read(ucons[0].ssc, &data, 1) != 1) {
		/* EOF, close socket and return last */
		close(ucons[0].ssc);
		ucons[0].ssc = 0;
		return -1;
	}

    return data;
}
static void scktsrv_out(BYTE data) {

	struct pollfd p[1];

	/* return if socket not connected */
	if (ucons[0].ssc == 0)
		return;

	/* if output not possible close socket and return */
	p[0].fd = ucons[0].ssc;
	p[0].events = POLLOUT;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLOUT)) {
		close(ucons[0].ssc);
		ucons[0].ssc = 0;
		return;
	}

again:
	if (write(ucons[0].ssc, &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			close(ucons[0].ssc);
			ucons[0].ssc = 0;
		}
	}
}

/* -------------------- MODEM HAL -------------------- */

#ifdef HAS_MODEM
#include "generic-at-modem.h"

static int modem_alive(void) {
    return modem_device_alive(0);
}
static void modem_status(BYTE *stat) {
    *stat &= (BYTE)(~3);
    if (modem_device_poll(0)) {
        *stat |= 2;
    }
    *stat |= 1;
}
static int modem_in(void) {
    return modem_device_get(0);
}
static void modem_out(BYTE data){
    modem_device_send(0, (char) data);
}
static int modem_cd(void) {
    return modem_device_carrier(0);
}
#endif /*HAS_MODEM*/

/* -------------------- HAL port/device mappings -------------------- */

const char *sio_port_name[MAX_SIO_PORT] = { "SIO1.portA", "SIO1.portB", "SIO2.portA", "SIO2.portB" };

static const hal_device_t devices[] = {
#ifdef HAS_NETSERVER
    { "WEBTTY", 0, net_tty_alive, net_tty_status, net_tty_in, net_tty_out, null_cd },
    { "WEBPTR", 0, net_ptr_alive, net_ptr_status, net_ptr_in, net_ptr_out, null_cd },
#else
    { "WEBTTY", 0, null_dead, null_status, null_in, null_out, null_cd },
    { "WEBPTR", 0, null_dead, null_status, null_in, null_out, null_cd },
#endif
    { "STDIO", 0, stdio_alive, stdio_status, stdio_in, stdio_out, null_cd },
    { "SCKTSRV", 0, scktsrv_alive, scktsrv_status, scktsrv_in, scktsrv_out, null_cd },
#ifdef HAS_MODEM
    { "MODEM", 0, modem_alive, modem_status, modem_in, modem_out, modem_cd },
#else
    { "MODEM", 0, null_dead, null_status, null_in, null_out, null_cd },
#endif
    { "VIOKBD", 0, vio_kbd_alive, vio_kbd_status, vio_kbd_in, vio_kbd_out, null_cd },
    { "", 0, null_alive, null_status, null_in, null_out, null_cd }
};

hal_device_t sio[MAX_SIO_PORT][MAX_HAL_DEV];

/* -------------------- HAL utility functions -------------------- */

static void hal_report(void) {

    int i, j;

    LOG(TAG, "\r\nSIO PORT MAP:\r\n");

    for (i = 0; i < MAX_SIO_PORT; i++) {

        LOG(TAG, "%s = ", sio_port_name[i]);
        j = 0;

        while (sio[i][j].name && j < MAX_HAL_DEV) {

            LOG(TAG, "%s%s", sio[i][j].name, (sio[i][j].fallthrough?"+":" "));

            j++;
        }
        LOG(TAG, "\r\n");
    }
}

static int hal_find_device(char *dev) {

    int i=0;
    while (i < MAX_HAL_DEV) {
        if (!strcmp(dev, devices[i].name)) {
            return i;
        }
        i++;
    }
    return -1;
}

static void hal_init(void) {

    int i, j, d;
    char *setting;
    char match[80];
    char *dev;

    /**
     *  Initialize HAL with default configuration, as follows:
     *
     *      SIO1.portA.device=WEBTTY,STDIO
     *      SIO1.portB.device=VIOKBD
     *      SIO2.portA.device=SCKTSRV
     *      SIO2.portB.device=MODEM
     *
     * Notes:
     *      - all ports end with NULL and that is always alive
     *      - the first HAL device in the list that is alive will service the request
     */
    sio[0][0] = devices[WEBTTYDEV];
    sio[0][1] = devices[STDIODEV];
    sio[0][2] = devices[NULLDEV];

    sio[1][0] = devices[VIOKBD];
    sio[1][1] = devices[NULLDEV];

    sio[2][0] = devices[WEBPTRDEV];
    sio[2][1] = devices[SCKTSRVDEV];
    sio[2][2] = devices[NULLDEV];

    sio[3][0] = devices[MODEMDEV];
    sio[3][1] = devices[NULLDEV];

    for (i = 0; i < MAX_SIO_PORT; i++) {

        j = 0;
        strcpy(match, sio_port_name[i]);
        strcat(match, ".device");

        if ((setting = getenv(match)) != NULL) {
            LOGI(TAG, "%s = %s", match, setting);

            strcpy(match, setting);

            dev = strtok(match, ",\r");
            while (dev) {

                char k = dev[strlen(dev) - 1];
                int fallthrough = 0;
                if (k == '+') {
                    dev[strlen(dev) - 1] = 0;
                    fallthrough = 1;
                }

                d = hal_find_device(dev);
                LOGI(TAG, "\tAdding %s to %s", dev, sio_port_name[i]);

                if (d >= 0) {
                    memcpy(&sio[i][j], &devices[d], sizeof(hal_device_t));
                    sio[i][j].fallthrough = fallthrough;
                    j++;
                }
                dev = strtok(NULL, ",\r");
            }
            memcpy(&sio[i][j], &devices[NULLDEV], sizeof(hal_device_t));
        }
    }
}

void hal_reset(void) {
    hal_init();
    hal_report();
}

/* -------------------- HAL - SIO interface -------------------- */

void hal_status_in(sio_port_t dev, BYTE *stat) {

    int p = 0;
    BYTE s;
    *stat = 0;
next:
    while(!sio[dev][p].alive()) { /* Find the first device that is alive */
        p++;
    }

    sio[dev][p].status(&s);
    *stat |= s;

    if (sio[dev][p].fallthrough) {
        p++;
        goto next;
    }
}

int hal_data_in(sio_port_t dev) {

    int p = 0;
    int in = 0;
next:
    while(!sio[dev][p].alive()) { /* Find the first device that is alive */
        p++;
    }

    in = sio[dev][p].in();

    if (in < 0 && sio[dev][p].fallthrough) {
        p++;
        goto next;
    } else {
        return in;
    }
}

void hal_data_out(sio_port_t dev, BYTE data) {

    int p = 0;
next:
    while(!sio[dev][p].alive()) { /* Find the first device that is alive */
        p++;
    }

	sio[dev][p].out(data);

    if (sio[dev][p].fallthrough) {
        p++;
        goto next;
    }
}

int hal_carrier_detect(sio_port_t dev) {

    int p = 0;
    while(!sio[dev][p].alive()) { /* Find the first device that is alive */
        p++;
    }

	return sio[dev][p].cd();
}
