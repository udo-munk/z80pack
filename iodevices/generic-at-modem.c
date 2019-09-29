/**
 * generic-at-modem.c
 * 
 * Emulation of generic 'AT' modem over TCP/IP sockets (telnet)
 *
 * Copyright (C) 2019 by David McNaughton
 * 
 * History:
 * 12-SEP-19    1.0     Initial Release
 */

#include <unistd.h>
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

#define AT_BUF_LEN 41
#define AUTOANSWER 1

static const char* TAG = "at-modem";

static int sfd = 0;
static int answer_sfd = 0;
static int newsockfd = 0;

static int *active_sfd = &sfd;

//TODO: pick some informed buffer lengths
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

        if (connect(sfd, rp->ai_addr, rp->ai_addr->sa_len) < 0) { 
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

    if ((answer_sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOGE(TAG, "Failed to create Answer socket");
        return 1;
    };
    if (setsockopt(answer_sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        LOGE(TAG, "Failed to setsockopt Answer socket");
        return 1;
    };

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8023);
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
    // write(newsockfd, "Hello\r\n", 7);
    return 0;
}

int answer_check_ring(void) {

	struct pollfd p[1];
    static int ringing = 0;
    static int rings = 0;
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

#ifndef AUTOANSWER
            if (!ringing) {
                gettimeofday(&ring_t1, NULL);
                ringing = 1;
                rings ++;
                LOGI(TAG, "Ringing %d", rings);
            }
#endif

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

#define MAX_HELP_LINE 11
char *at_help[MAX_HELP_LINE] = {
			LF "AT COMMANDS:" CRLF,
			"AT  - 'AT' Test" CRLF,
			"AT? - Help" CRLF,
			"A/  - (immediate) Repeat last" CRLF,
			"ATZ - Reset modem" CRLF,
			"ATDhostname:port - Dial hostname, port optional (default:23)" CRLF,
			"+++ - Return to command mode" CRLF,
			"ATO - Return to data mode" CRLF,
			"ATH - Hangup" CRLF,
			"ATL - Listen for incoming calls" CRLF,
			"ATA - Answer" CRLF

};

void at_cat_c(char c) {
    if (strlen(at_cmd) >= AT_BUF_LEN - 1) {
        LOGE(TAG, "Buffer overflow");
        return;
    }

	if (c == '\b') {
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

int process_at_cmd(void) {
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
        //TODO: add a command to check connection status
		case '\r': // AT<CR>
			break;
		case '?': // AT?
            help_line = 0;
            at_state = help;
			break;
		case 'Z': // ATZ
            // at_prev[0] = 0;
			// at_cat_s(LF "RESET" CR);
			// AT_END_CMD;
			// break;
            // Intentional fall-through to ATH for now ie. ATZ = ATH
		case 'H': // ATH
            if (*active_sfd) {
                close_socket();
			    at_cat_s(LF "HANGUP" CR);
            }
			break;
		case 'D': // ATD
            //TODO: check and repond if Wi-Fi STA up : ie. NO DIAL TONE

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
		case 'O': // ATO
			AT_END_CMD;
            if (*active_sfd == 0) {
                strcpy(at_err, LF AT_NO_CARRIER CRLF);
                return 1;
            }
            at_state = dat;
			// at_cat_s(LF "CONNECT" CR);
			break;
        case 'L': // ATL - listen
            AT_END_CMD;
            answer_init();
            break;
        case 'A': // ATA - answer
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
        // LOGI("+++", "Ind [%s](%d)", at_buf, strlen(at_buf));
        if (strlen(at_buf) == 3) {
        	gettimeofday(&at_t2, NULL);
            tdiff = time_diff_sec(&at_t1, &at_t2);
            // LOGI("+++", "TDIFF %d", tdiff);
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
#ifndef AUTOANSWER
            at_cat_s(CRLF AT_RING);
#else
            if (!answer()) {
                at_state = dat;
            } 
#endif
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
            //TODO: this will occur if the socket is disconnected
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
	// LOGW(TAG, "SIO2: [%d][%c]", data, data);

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
            // NO break;
            // Intentional fallthrough here.
        case dat:
        	gettimeofday(&at_t2, NULL);
            tdiff = time_diff_sec(&at_t1, &at_t2);
            // if (data == '+') LOGI("+", "Time since last char %d", tdiff);

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