/**
 * generic-at-modem.c
 * 
 * Emulation of generic 'AT' modem over TCP/IP sockets (telnet)
 *
 * Copyright (C) 2019 by David McNaughton
 * 
 * History:
 * 12-SEP-19    1.0     Initial Release
 * 29-SEP-19    1.1     Added Answer modes and registers
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "log.h"

#define AT_BUF_LEN      41
#define LISTENER_PORT   8023

static const char* TAG = "at-modem";

static int sfd = 0;
static int answer_sfd = 0;
static int newsockfd = 0;

static int *active_sfd = &sfd;

static char addr[AT_BUF_LEN];
static char port_num[10];

int time_diff_sec(struct timeval *, struct timeval *);

int open_socket(void) {

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    void *addrptr = NULL;
    uint16_t port = 0;
    struct sockaddr_in *sktv4 = NULL;
    struct sockaddr_in6 *sktv6 = NULL;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow only IPv4 not IPv6 */
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(addr, port_num, &hints, &result);
    if (s != 0) {
        LOGE(TAG, "getaddrinfo: %s\n", "failed");
        return 1;
    }

   for (rp = result; rp != NULL; rp = rp->ai_next) {

        LOGD(TAG, "Address: %d", rp->ai_family);
        switch (rp->ai_family) {
            case AF_INET:
                sktv4 = (struct sockaddr_in *)rp->ai_addr;
                port = sktv4->sin_port;
                addrptr = &sktv4->sin_addr;
                break;
            case AF_INET6:
                sktv6 = (struct sockaddr_in6 *)rp->ai_addr;
                port = sktv6->sin6_port;
                addrptr = &sktv6->sin6_addr;
                LOGE(TAG, "Not expecting IPV6 addresses");
                return 1;
                break;
            default:
                LOGE(TAG, "Not expecting address family type %d", rp->ai_family);
                return 1;
                break;
        }

        inet_ntop(rp->ai_family, addrptr, addr, 100);
        LOGI(TAG, "Address: %s:%d", addr, ntohs(port));

        if ((sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0) {
            LOGE(TAG, "Failed to create socket");
            return 1;
        };

        if (connect(sfd, rp->ai_addr, sizeof(struct sockaddr_in)) < 0) { 
            LOGE(TAG, "Failed to connect to socket: %d", errno);
            return 1;
        } 

        LOGI(TAG, "Socket connected");
        active_sfd = &sfd;
        return 0;
    }

    return 1;
}

void close_socket(void) {

    if (shutdown(*active_sfd, SHUT_RDWR) == 0) {
        LOGI(TAG, "Socket shutdown");
    }
    if (close(*active_sfd) == 0) {
        LOGI(TAG, "Socket closed");
        *active_sfd = 0;
    }
}

/****************************************************************************************************************************/

int answer_init(void) {

    struct sockaddr_in serv_addr;
    int enable = 1;

    if (answer_sfd) {
        LOGE(TAG, "Already listening");
        return 1;
    }

    if ((answer_sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOGE(TAG, "Failed to create Answer socket");
        answer_sfd = 0;
        return 1;
    };
    if (setsockopt(answer_sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        LOGE(TAG, "Failed to setsockopt Answer socket");
        return 1;
    };

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(LISTENER_PORT);
    if (bind(answer_sfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        LOGE(TAG, "ERROR on binding %d %s", errno, strerror(errno));
        return 1;
    }
    listen(answer_sfd,1);
    inet_ntop(AF_INET, &serv_addr.sin_addr, addr, 100);
    LOGI(TAG, "Listening on %s:%d", addr, ntohs(serv_addr.sin_port));
    return 0;
}

int answer(void) {

    struct sockaddr_in cli_addr;
    socklen_t clilen;

    clilen = sizeof(cli_addr);
    newsockfd = accept(answer_sfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        LOGE(TAG, "ERROR on accept");
        return 1;
    }

    inet_ntop(AF_INET, &cli_addr.sin_addr, addr, 100);
    LOGI(TAG, "New Remote Connection: %s:%d", addr, ntohs(cli_addr.sin_port));
    active_sfd = &newsockfd;
    return 0;
}

int answer_check_ring(void) {

	struct pollfd p[1];
    static int ringing = 0;
    static struct timeval ring_t1, ring_t2;
    int tdiff;
    
    if (answer_sfd) {

        if (ringing) {
            gettimeofday(&ring_t2, NULL);
            tdiff = time_diff_sec(&ring_t1, &ring_t2);
            if (tdiff < 3) {
                return 0;
            } else {
                ringing = 0;
            }
        }

        p[0].fd = answer_sfd;
        p[0].events = POLLIN;
        p[0].revents = 0;
        poll(p, 1, 0);

        if (p[0].revents == POLLIN) {

            if (!ringing) {
                gettimeofday(&ring_t1, NULL);
                ringing = 1;
                LOGI(TAG, "Ringing");
            }
            return 1;
        }
    } 
    return 0;
}

/****************************************************************************************************************************/

char at_buf[AT_BUF_LEN * 4] = "";
char at_cmd[AT_BUF_LEN] = "";
char at_prev[AT_BUF_LEN] = "";
char at_err[AT_BUF_LEN] = "";

char *at_out = at_buf;
enum at_states { cmd, A_recv, AT_recv, AS_recv, dat, intr, help };
typedef enum at_states at_state_t;

at_state_t at_state = cmd;
int help_line = 0;

#define CR			"\r"
#define LF			"\n"
#define CRLF		CR LF

#define AT_OK		    "OK" CRLF
#define AT_ERROR	    "ERROR" CRLF
#define AT_NO_ANSWER	"NO ANSWER" CRLF
#define AT_NO_CARRIER	"NO CARRIER" CRLF
#define AT_NO_DIALTONE	"NO DIALTONE" CRLF
#define AT_RING	        "RING" CRLF

#define MAX_HELP_LINE 14
char *at_help[MAX_HELP_LINE] = {
			LF "AT COMMANDS:" CRLF,
			"AT  - 'AT' Test" CRLF,
			"AT$ - Help" CRLF,
			"A/  - (immediate) Repeat last" CRLF,
			"ATZ - Reset modem" CRLF,
			"ATDhostname:port - Dial hostname, port optional (default:23)" CRLF,
			"+++ - Return to command mode" CRLF,
			"ATO - Return to data mode" CRLF,
			"ATH - Hangup" CRLF,
			"AT&A - Enable Answer mode - listen for incoming calls" CRLF,
			"ATA - Answer" CRLF,
			"ATSn - Select register n" CRLF,
			"AT? - Query current register" CRLF,
			"AT=r - Set current register to r" CRLF
};

void at_cat_c(char c) {
    if (strlen(at_cmd) >= AT_BUF_LEN - 1) {
        LOGE(TAG, "Buffer overflow");
        return;
    }

	if (c == '\b') { /*TODO: use s_reg[SREG_BS] */
		if(strlen(at_cmd) > 2) {
			at_buf[strlen(at_buf) + 3] = 0;
			at_buf[strlen(at_buf) + 2] = c;
			at_buf[strlen(at_buf) + 1] = ' ';
			at_buf[strlen(at_buf)] = c;
			at_cmd[strlen(at_cmd) - 1] = 0;
		}
	} else {
		at_buf[strlen(at_buf) + 1] = 0;
		at_buf[strlen(at_buf)] = c;
		at_cmd[strlen(at_cmd) + 1] = 0;
		at_cmd[strlen(at_cmd)] = c;
	}
	LOGD(TAG, "AT CMD: [%s]", at_cmd);
}

void at_cat_s(char *s) {
    if ((strlen(at_cmd) + strlen(s)) >= (AT_BUF_LEN*4) - 1) {
        LOGE(TAG, "Buffer overflow");
        return;
    }

	strcat(at_buf, s);
}

#define SREG_AA     0
#define SREG_RINGS  1
#define SREG_ESCAPE 2
#define SREG_CR     3
#define SREG_LF     4
#define SREG_BS     5
#define MAX_REG_NUM 13

#define SREG_DEFAULTS { 0, 0, 43, 13, 10, 8 }

static unsigned int s_reg[MAX_REG_NUM] = SREG_DEFAULTS;
static unsigned int s_reg_defaults[MAX_REG_NUM] = SREG_DEFAULTS;
static int reg = 0;

int process_at_cmd(void) {
    int tmp_reg;
	char *at_ptr = at_cmd;
    char *arg_ptr;
	strcpy(at_prev, at_cmd);

	if ((*at_ptr++ =='A') && (*at_ptr++ == 'T')) {
		at_cmd[0] = 0;
	} else {
		at_cmd[0] = 0;
		strcpy(at_err, LF AT_ERROR CRLF);
		return 1;
	}

#define AT_END_CMD	(at_ptr = &at_cmd[strlen(at_cmd)])

  while (*at_ptr != 0) {
	switch (*at_ptr++) {
        /*TODO: add a command to check connection status */
		case '\r': /* AT<CR> */ /*TODO: use s_reg[SREG_CR] */
			break;
		case '$': /* AT$ */
            help_line = 0;
            at_state = help;
			break;
		case 'Z': /* ATZ */
            memset(at_prev, 0, sizeof(at_prev));
            for (tmp_reg = 0; tmp_reg < MAX_REG_NUM; tmp_reg++) {
                s_reg[tmp_reg] = s_reg_defaults[tmp_reg];
            }
            if (*active_sfd) {
                close_socket();
            }
            if (answer_sfd) {
                active_sfd = &answer_sfd;
                close_socket();
                active_sfd = &sfd;

            }
			AT_END_CMD;
            break;
		case 'H': /* ATH */
            if (*active_sfd) {
                s_reg[SREG_RINGS] = 0;
                close_socket();
			    at_cat_s(LF "HANGUP" CR);
            }
			break;
		case 'D': /* ATDaddr[:port=23] */
            /*TODO: check and repond if Wi-Fi STA up : ie. NO DIAL TONE */

            if (*active_sfd) {
                strcpy(at_err, LF "ALREADY IN CALL" CRLF);
			    return 1;
            }

            strcpy(at_err, at_ptr);
            arg_ptr = strtok(at_err, ":\r");

            if(arg_ptr == NULL) {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }

            strncpy(addr, arg_ptr, AT_BUF_LEN - 1);
            LOGI(TAG, "Addr: [%s]", addr);
            arg_ptr =  strtok(NULL, "\r");
            if (arg_ptr != NULL) {
                strncpy(port_num, arg_ptr, 10);
            } else {
                strcpy(port_num, "23");
            }
            LOGI(TAG, "Port: [%s]", port_num);

            if (open_socket()) {
                strcpy(at_err, LF AT_NO_ANSWER CRLF);
                return 1;
            };
			AT_END_CMD;
            at_state = dat;
			at_cat_s(LF "CONNECT" CR);
			break;
		case 'S': /* ATSn */
            tmp_reg = strtod(at_ptr, &arg_ptr);

            if (tmp_reg < 0 || tmp_reg > (MAX_REG_NUM - 1) || arg_ptr == at_ptr) {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }

            reg = tmp_reg;
            LOGI(TAG, "AT Register set to %d", reg);
            at_ptr = arg_ptr;
			break;
		case '?': /* AT? */
            at_ptr++;
            LOGI(TAG, "ATS%d? is %d", reg, s_reg[reg]);
            sprintf(at_err, LF "%d" CRLF, s_reg[reg]);
            at_cat_s(at_err);
			break;
		case '=': /* AT=r */
            tmp_reg = strtod(at_ptr, &arg_ptr);

            if (tmp_reg < 0 || tmp_reg > 255 || arg_ptr == at_ptr) {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }

            LOGI(TAG, "ATS%d = %d", reg, tmp_reg);
            at_ptr = arg_ptr;
            s_reg[reg] = tmp_reg;
			break;
		case 'O': /* ATO */
			AT_END_CMD;
            if (*active_sfd == 0) {
                strcpy(at_err, LF AT_NO_CARRIER CRLF);
                return 1;
            }
            at_state = dat;
			break;
        case '&': /* AT&A - enable answer - listen */
            if (*at_ptr == 'A') {
                AT_END_CMD;
                if (answer_init()) {
                    strcpy(at_err, LF AT_ERROR CRLF);
                    return 1;
                };
            } else {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }
            break;
        case 'A': /* ATA - answer */
            answer();
            AT_END_CMD;
            at_state = dat;
            break;
		default:
			strcpy(at_err, LF AT_ERROR CRLF);
			return 1;
			break;
	}
  }

	return 0;
}

int time_diff_sec(struct timeval *t1, struct timeval *t2)
{
	long sec, usec;

	sec = (long) t2->tv_sec - (long) t1->tv_sec;
	usec = (long) t2->tv_usec - (long) t1->tv_usec;
	/* normalize result */
	if (usec < 0L) {
		sec--;
		usec += 1000000L;
	}
    return sec;
}

static struct timeval at_t1, at_t2;
int tdiff;

int modem_device_alive(int i) {

    i = i;
    return 1;
}

int modem_device_poll(int i) {

	struct pollfd p[1];

    i = i;

    if (at_state == help) {
        if (strlen(at_buf) == 0) {
            strcpy(at_buf, at_help[help_line++]);
            if (help_line == MAX_HELP_LINE) {
                at_state = cmd;
            }
        }
        return (strlen(at_out) > 0);
    } else if (at_state == intr) {
        if (strlen(at_buf) == 3) {
        	gettimeofday(&at_t2, NULL);
            tdiff = time_diff_sec(&at_t1, &at_t2);
            if (tdiff > 0) {
                at_state = cmd;
                LOGI(TAG, "+++ Returning to CMD mode");
                at_cat_s(CRLF AT_OK);
                return (strlen(at_buf) > 0);
            }
        }
        return 0;
    } else if (at_state == dat) {
        p[0].fd = *active_sfd;
        p[0].events = POLLIN | POLLOUT;
        p[0].revents = 0;
        poll(p, 1, 0);
        return (p[0].revents & POLLIN);
    } else {
        if (answer_check_ring()) {
            s_reg[SREG_RINGS]++;
            if ((s_reg[SREG_AA] > 0) && (s_reg[SREG_RINGS] > s_reg[SREG_AA])) {
                if (!answer()) {
                    at_state = dat;
                } 
            } else {
                at_cat_s(CRLF AT_RING);
            }
        }

        return (strlen(at_out) > 0);
    }
}

int modem_device_get(int i) {

	struct pollfd p[1];
    unsigned char data;

    i = i;

    if (at_state == dat) {

        p[0].fd = *active_sfd;
        p[0].events = POLLIN;
        p[0].revents = 0;
        poll(p, 1, 0);
        if (!(p[0].revents & POLLIN))
            return -1;

        if (read(*active_sfd, &data, 1) == 0) {
            /* this will occur if the socket is disconnected */
            LOGI(TAG, "Socket disconnected");
            at_buf[0] = 0;
            at_cat_s(CRLF AT_NO_CARRIER);
            at_cmd[0] = 0;
            at_state = cmd;

            return -1;
        }

    } else {
    
        if (strlen(at_out) > 0) {
            data = *at_out;
            at_out++;
            if (strlen(at_out) == 0) {
                at_out = at_buf;
                at_buf[0] = 0;
            }
        } else {
            return -1;
        }
    }

    return data;
}

void modem_device_send(int i, char data) {

    i = i;

	switch (at_state) {
    /***
     *  data mode
     */
        case intr:
            if (data == '+' && strlen(at_buf) < 3) {
                at_buf[strlen(at_buf)+1] = 0;
                at_buf[strlen(at_buf)] = data;
        	    gettimeofday(&at_t1, NULL);
                return;
            } else {
                write(*active_sfd, at_buf, strlen(at_buf));
                at_state = dat;
            }
            /***
             * NO break;
             * Intentional fallthrough here.
             */
        case dat:
        	gettimeofday(&at_t2, NULL);
            tdiff = time_diff_sec(&at_t1, &at_t2);

            if (data == '+' && (tdiff > 0)) {
                at_buf[0] = data;
                at_buf[1] = 0;
                at_out = at_buf;
                at_state = intr;
                return;
            }
            write(*active_sfd, (char *) &data, 1);
            break;
	/***
	 * AT command mode
	 */
		case cmd:
			at_cat_c(data);
			if (data == 'A') {
				at_state = A_recv;
			} else {
				at_cat_s(CRLF AT_ERROR);
				at_cmd[0] = 0;
				at_state = cmd;
			}
			break;
		case A_recv:
			at_cat_c(data);
			if (data == 'T') {
				at_state = AT_recv;
			} else if (data == '/') {
				at_state = AS_recv;
				at_cat_s(CR);
				strcpy(at_cmd, at_prev);
				if (process_at_cmd()) {
					at_cat_s(at_err);
				} else {
					at_cat_s(LF AT_OK);
				}
				at_state = cmd;
			} else {
				at_cat_s(CRLF AT_ERROR);
				at_cmd[0] = 0;
				at_state = cmd;
			}
			break;
		case AT_recv:
			at_cat_c(data);
			if (data == '\r') {
				at_state = cmd;
				if (process_at_cmd()) {
					at_cat_s(at_err);
				} else {
					at_cat_s(LF AT_OK);
				}
			}
			break;
		case AS_recv:
			LOGE(TAG, "A/: Didn't expect to get here");
			break;
		default:
			LOGE(TAG, "AT statemachine unknown [%d]", at_state);
		break;
	}

    if (at_state == dat) gettimeofday(&at_t1, NULL);
}
