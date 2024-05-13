/**
 * cromemco-d+7a.c
 * 
 * Emulation of the Cromemco D+7A I/O 
 *
 * Copyright (C) 2020 by David McNaughton
 * 
 * History:
 * 14-JAN-2020	1.0	Initial Release
 */
#include <stdlib.h>
#include <stdio.h>
#include "sim.h"
#include "simglb.h"

#ifdef HAS_NETSERVER
#include "netsrv.h"
#endif
// #define LOG_LOCAL_LEVEL LOG_DEBUG
#include "log.h"

static const char *TAG = "D+7AIO";

#define PORT_COUNT  8

BYTE inPort[PORT_COUNT];
BYTE outPort[PORT_COUNT];

void cromemco_d7a_callback(BYTE *data) {
    
    int i;
    inPort[0] = *data++;
    for (i=1; i < PORT_COUNT; i++) {
        inPort[i] = (*data++) - 128;
    }
}

void cromemco_d7a_init(void) {

    inPort[0] = 0xFF;

#ifdef HAS_NETSERVER
    if (n_flag) {
        net_device_service(DEV_D7AIO, cromemco_d7a_callback);
    }
#endif

}

void cromemco_d7a_out(BYTE port, BYTE data)
{
    outPort[port] = data;

    LOGD(TAG, "Output %d on port %d", data, port);

#ifdef HAS_NETSERVER
    if (n_flag) {
        // if (net_device_alive(DEV_D7AIO)) {
            net_device_send(DEV_D7AIO, (char *)&data, 1);
        // }
    }
#endif
}

void cromemco_d7a_D_out(BYTE data) { cromemco_d7a_out(0, data); }
void cromemco_d7a_A1_out(BYTE data) { cromemco_d7a_out(1, data); }
void cromemco_d7a_A2_out(BYTE data) { cromemco_d7a_out(2, data); }
void cromemco_d7a_A3_out(BYTE data) { cromemco_d7a_out(3, data); }
void cromemco_d7a_A4_out(BYTE data) { cromemco_d7a_out(4, data); }
void cromemco_d7a_A5_out(BYTE data) { cromemco_d7a_out(5, data); }
void cromemco_d7a_A6_out(BYTE data) { cromemco_d7a_out(6, data); }
void cromemco_d7a_A7_out(BYTE data) { cromemco_d7a_out(7, data); }

BYTE cromemco_d7a_in(BYTE port)
{
	return inPort[port];
}

BYTE cromemco_d7a_D_in(void) { return cromemco_d7a_in(0); };
BYTE cromemco_d7a_A1_in(void) { return cromemco_d7a_in(1); };
BYTE cromemco_d7a_A2_in(void) { return cromemco_d7a_in(2); };
BYTE cromemco_d7a_A3_in(void) { return cromemco_d7a_in(3); };
BYTE cromemco_d7a_A4_in(void) { return cromemco_d7a_in(4); };
BYTE cromemco_d7a_A5_in(void) { return cromemco_d7a_in(5); };
BYTE cromemco_d7a_A6_in(void) { return cromemco_d7a_in(6); };
BYTE cromemco_d7a_A7_in(void) { return cromemco_d7a_in(7); };
