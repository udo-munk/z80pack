/*
 * This module is based on the NTP client example provided by Raspberry Pi.
 * The example code is reformatted and modified for the needs in this project.
 *
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * History:
 * 16-MAY-2025 first implementation
 */

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "net_vars.h"

typedef struct NTP_T_ {
	ip_addr_t ntp_server_address;
	bool dns_request_sent;
	struct udp_pcb *ntp_pcb;
	absolute_time_t ntp_test_time;
	alarm_id_t ntp_resend_alarm;
} NTP_T;

#define NTP_MSG_LEN	48
#define NTP_PORT	123
#define NTP_DELTA	2208988800 /* seconds between 1 Jan 1900 and 1 Jan 1970 */
#define NTP_TEST_TIME	(30 * 1000)
#define NTP_RESEND_TIME	(10 * 1000)

static volatile bool ntp_done = false;
time_t ntp_time;

/*
 * Called with results of operation
 */
static void ntp_result(NTP_T *state, int status, time_t *result)
{
	if (status == 0 && result) {
		struct tm *utc = gmtime(result);
		memcpy(&ntp_time, result, sizeof(time_t));
		printf("got ntp response: %02d/%02d/%04d %02d:%02d:%02d UTC\n",
		       utc->tm_mday, utc->tm_mon + 1, utc->tm_year + 1900,
		       utc->tm_hour, utc->tm_min, utc->tm_sec);
	}

	if (state->ntp_resend_alarm > 0) {
		cancel_alarm(state->ntp_resend_alarm);
		state->ntp_resend_alarm = 0;
	}
	state->ntp_test_time = make_timeout_time_ms(NTP_TEST_TIME);
	state->dns_request_sent = false;
	ntp_done = true;
}

/*
 * Make an NTP request
 */
static void ntp_request(NTP_T *state)
{
	cyw43_arch_lwip_begin();
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
	uint8_t *req = (uint8_t *) p->payload;
	memset(req, 0, NTP_MSG_LEN);
	req[0] = 0x1b;
	udp_sendto(state->ntp_pcb, p, &state->ntp_server_address, NTP_PORT);
	pbuf_free(p);
	cyw43_arch_lwip_end();
}

static int64_t ntp_failed_handler(__attribute__((unused)) alarm_id_t id, void *user_data)
{
	NTP_T *state = (NTP_T *) user_data;

	puts("ntp request failed");
	ntp_result(state, -1, NULL);
	return 0;
}

/*
 * Call back with a DNS result
 */
static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
	NTP_T *state = (NTP_T *) arg;

	if (ipaddr) {
		state->ntp_server_address = *ipaddr;
		printf("got ip address %s for ntp server %s\n", ipaddr_ntoa(ipaddr), hostname);
		ntp_request(state);
	} else {
		puts("ntp dns request failed");
		ntp_result(state, -1, NULL);
	}
}

/*
 * NTP data received
 */
static void ntp_recv(void *arg, __attribute__((unused)) struct udp_pcb *pcb, struct pbuf *p,
		     const ip_addr_t *addr, u16_t port)
{
	NTP_T *state = (NTP_T *) arg;
	uint8_t mode = pbuf_get_at(p, 0) & 0x7;
	uint8_t stratum = pbuf_get_at(p, 1);

	/* Check the result */
	if (ip_addr_cmp(addr, &state->ntp_server_address) && port == NTP_PORT &&
	    p->tot_len == NTP_MSG_LEN && mode == 0x4 && stratum != 0) {
		uint8_t seconds_buf[4] = {0};
		pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
		uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 |
			 seconds_buf[2] << 8 | seconds_buf[3];
		uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;
		time_t epoch = seconds_since_1970;
		ntp_result(state, 0, &epoch);
	} else {
		puts("invalid ntp response");
		ntp_result(state, -1, NULL);
	}
	pbuf_free(p);
}

/*
 * Perform initialisation
 */
static NTP_T *ntp_init(void)
{
	NTP_T *state = (NTP_T *) calloc(1, sizeof(NTP_T));

	if (!state) {
		puts("failed to allocate state");
		return NULL;
	}
	state->ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
	if (!state->ntp_pcb) {
		puts("failed to create pcb");
		free(state);
		return NULL;
	}
	udp_recv(state->ntp_pcb, ntp_recv, state);
	return state;
}

/*
 * Get time and date
 */
void do_ntp(void)
{
	NTP_T *state = ntp_init();
	if (!state)
		return;

	if (absolute_time_diff_us(get_absolute_time(), state->ntp_test_time) < 0 &&
	    !state->dns_request_sent) {
		/* Set alarm in case udp requests are lost */
		state->ntp_resend_alarm = add_alarm_in_ms(NTP_RESEND_TIME, ntp_failed_handler,
							  state, true);
		cyw43_arch_lwip_begin();
		int err = dns_gethostbyname((const char *) &ntp_server,
					    &state->ntp_server_address, ntp_dns_found, state);
		cyw43_arch_lwip_end();
		state->dns_request_sent = true;
		if (err == ERR_OK) {
			ntp_request(state); /* Cached result */
		} else if (err != ERR_INPROGRESS) { /* ERR_INPROGRESS means expect a callback */
			puts("dns request failed");
			ntp_result(state, -1, NULL);
		}
	}
	free(state);

	while (ntp_done == false)
		sleep_ms(100);
}
