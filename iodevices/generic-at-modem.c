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
 * 20-OCT-19    1.2     Added Telnet handler
 * 23-OCT-19    1.3     Put Telnet protocol under modem register control
 */

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "libtelnet.h"

#define LOG_LOCAL_LEVEL LOG_WARN
#include "log.h"

#define AT_BUF_LEN      41
#define DEFAULT_LISTENER_PORT   8023
#define _QUOTE(arg)      #arg
#define STR_VALUE(arg)    _QUOTE(arg)

static const char* TAG = "at-modem";

#define SREG_AA     0
#define SREG_RINGS  1
#define SREG_ESCAPE 2
#define SREG_CR     3
#define SREG_LF     4
#define SREG_BS     5
#define SREG_PORT   14
#define SREG_TELNET 15
#define SREG_TN_SGA 16
#define SREG_TN_ECHO 17
#define SREG_TN_BIN 18
#define SREG_TN_NAWS 19
#define SREG_COLS   20
#define SREG_ROWS   21
#define SREG_TN_TTYPE 22
#define MAX_REG_NUM 25

#define SREG_DEFAULTS { \
    /* SREG_AA */       0, \
    /* SREG_RINGS */    0, \
    /* SREG_ESCAPE */   43, \
    /* SREG_CR */       13, \
    /* SREG_LF */       10, \
    /* SREG_BS */       8, \
    0, 0, 0, 0, 0, 0, 0, 0, \
    /* SREG_PORT */     DEFAULT_LISTENER_PORT, \
    /* SREG_TELNET */   0, \
    /* SREG_TN_SGA */   3, \
    /* SREG_TN_ECHO */  0, \
    /* SREG_TN_BIN */   0, \
    /* SREG_TN_NAWS */  3, \
    /* SREG_COLS */     80, \
    /* SREG_ROWS */     24, \
    /* SREG_TN_TTYPE */ 3, \
};

static unsigned int s_reg[MAX_REG_NUM] = SREG_DEFAULTS;
static unsigned int s_reg_defaults[MAX_REG_NUM] = SREG_DEFAULTS;
static int reg = 0;

static int sfd = 0;
static int answer_sfd = 0;
static int newsockfd = 0;

static int *active_sfd = &sfd;

static char addr[AT_BUF_LEN];
static char port_num[10];

static telnet_telopt_t telnet_opts[10];

void init_telnet_opts(void) {

    int i=0;

    telnet_opts[i].telopt = TELNET_TELOPT_SGA;
    telnet_opts[i].us =  (s_reg[SREG_TN_SGA] & 1)? TELNET_WILL : TELNET_WONT;
    telnet_opts[i].him = (s_reg[SREG_TN_SGA] & 2)? TELNET_DO : TELNET_DONT;
    i++;
    telnet_opts[i].telopt = TELNET_TELOPT_ECHO;
    telnet_opts[i].us =  (s_reg[SREG_TN_ECHO] & 1)? TELNET_WILL : TELNET_WONT;
    telnet_opts[i].him = (s_reg[SREG_TN_ECHO] & 2)? TELNET_DO : TELNET_DONT;
    i++;
    telnet_opts[i].telopt = TELNET_TELOPT_BINARY;
    telnet_opts[i].us =  (s_reg[SREG_TN_BIN] & 1)? TELNET_WILL : TELNET_WONT;
    telnet_opts[i].him = (s_reg[SREG_TN_BIN] & 2)? TELNET_DO : TELNET_DONT;
    i++;
    telnet_opts[i].telopt = TELNET_TELOPT_NAWS;
    telnet_opts[i].us =  (s_reg[SREG_TN_NAWS] & 1)? TELNET_WILL : TELNET_WONT;
    telnet_opts[i].him = (s_reg[SREG_TN_NAWS] & 2)? TELNET_DO : TELNET_DONT;
    i++;
    telnet_opts[i].telopt = TELNET_TELOPT_TTYPE;
    telnet_opts[i].us =  (s_reg[SREG_TN_TTYPE] & 1)? TELNET_WILL : TELNET_WONT;
    telnet_opts[i].him = (s_reg[SREG_TN_TTYPE] & 2)? TELNET_DO : TELNET_DONT;
    i++;
    telnet_opts[i].telopt = -1;
    telnet_opts[i].us =  0;
    telnet_opts[i].him = 0;
}

#define TELNET_TTYPE   "ansi"

static telnet_t *telnet =  NULL;
static unsigned char tn_recv;
static int tn_len = 0;

int time_diff_sec(struct timeval *, struct timeval *);

static void telnet_hdlr(telnet_t *telnet, telnet_event_t *ev, void *user_data) {

    char buf[81];
    char * p;
    int i;

    user_data = user_data;

    switch (ev->type) {
    case TELNET_EV_DATA:
		if (ev->data.size) {
            if (ev->data.size != 1) {
                LOGW(TAG, "LONGER THAN EXPECTED [%ld]", ev->data.size);
            } else {
                tn_recv = *(ev->data.buffer);
                tn_len = ev->data.size;
            }
            LOGD(TAG, "Telnet IN: %c[%ld]", tn_recv, ev->data.size);
		}
		break;
    case TELNET_EV_SEND:
		if (ev->data.size) {
            p = buf;
            for(i=0;i<(int)ev->data.size;i++) {
                p += sprintf (p, "%d ", *(ev->data.buffer+i));
            }
            LOGD(TAG, "Telnet OUT: %s[%ld]", buf, ev->data.size);
		}
		write(*active_sfd, ev->data.buffer, ev->data.size);
		break;
    case TELNET_EV_WILL:
        if (ev->neg.telopt == TELNET_TELOPT_SGA) {
            LOGI(TAG, "Telnet WILL SGA");
        } else if (ev->neg.telopt == TELNET_TELOPT_BINARY) {
            LOGI(TAG, "Telnet WILL BINARY");
        } else {
            LOGW(TAG, "Telnet WILL unknown opt: %d", ev->neg.telopt);
        }
		break;
    case TELNET_EV_DO:
        if (ev->neg.telopt == TELNET_TELOPT_SGA) {
            LOGI(TAG, "Telnet DO SGA");
        } else if (ev->neg.telopt == TELNET_TELOPT_BINARY) {
            LOGI(TAG, "Telnet DO BINARY");
        } else if (ev->neg.telopt == TELNET_TELOPT_TTYPE) {
            LOGI(TAG, "Telnet DO TTYPE");
        } else if (ev->neg.telopt == TELNET_TELOPT_NAWS) {
            telnet_begin_sb(telnet, TELNET_TELOPT_NAWS);
            buf[0] = 0; 
            buf[1] = s_reg[SREG_COLS]; 
            buf[2] = 0; 
            buf[3] = s_reg[SREG_ROWS]; 
            telnet_send(telnet, buf, 4);
            telnet_finish_sb(telnet);
            LOGI(TAG, "Telnet DO NAWS [%d x %d]", (buf[0]<<8) + buf[1], (buf[2]<<8) + buf[3]);
        } else {
            LOGW(TAG, "Telnet DO unknown opt: %d", ev->neg.telopt);
        }
        break;
    case TELNET_EV_SUBNEGOTIATION:
		LOGI(TAG, "SUBNEGOTIATION [%d]", ev->sub.telopt);
        break;
    case TELNET_EV_TTYPE:
		LOGI(TAG, "TTYPE negotiation cmd:%d", ev->ttype.cmd);
        char *ttype;
        if (((ttype = getenv("TERM")) == NULL) || (s_reg[SREG_TN_TTYPE] & 4)) {
            ttype = TELNET_TTYPE;
        }
        if (ev->ttype.cmd == TELNET_TTYPE_SEND) {
		    LOGI(TAG, "TTYPE SEND : %s", ttype);
            telnet_ttype_is(telnet, ttype);
        } else if (ev->ttype.cmd == TELNET_TTYPE_IS) {
		    LOGI(TAG, "TTYPE IS : %s", ev->ttype.name);
        }
        break;
    case TELNET_EV_ERROR:
		LOGE(TAG, "ERROR: %s", ev->error.msg);
        break;
    default:
        LOGW(TAG, "Telnet Event unknown [%d] opt: %d", ev->type, ev->neg.telopt);
        break;
    }
}

/****************************************************************************************************************************/

int open_socket(void) {

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    void *addrptr = NULL;
    uint16_t port = 0;
    struct sockaddr_in *sktv4 = NULL;
    struct sockaddr_in6 *sktv6 = NULL;
    int on = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow only IPv4 not IPv6 */
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(addr, port_num, &hints, &result);
    if (s != 0) {
        LOGI(TAG, "getaddrinfo: %s\n", "failed");
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
            LOGI(TAG, "Failed to connect to socket: %d", errno);
            return 1;
        }

        if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *) &on, sizeof(on)) == -1) {
	        LOGE(TAG, "can't set sockopt TCP_NODELAY");
	        return 1;
	    }

        LOGI(TAG, "Socket connected");
        active_sfd = &sfd;

        /* Initialise Telnet session */
        if (s_reg[SREG_TELNET]) {
            init_telnet_opts();
            if ((telnet = telnet_init(telnet_opts, telnet_hdlr, 0, NULL)) == 0) {
                LOGE(TAG, "can't initialise telnet session");
                return 1;
            } else {
                LOGI(TAG, "Telnet session started");
            };
        };

        return 0;
    }

    return 1;
}

void close_socket(void) {

    if (telnet != NULL) {
        telnet_free(telnet);
        telnet = NULL;
        LOGI(TAG, "Telnet session ended");
    }

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
    serv_addr.sin_port = htons(s_reg[SREG_PORT]);
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
    int on = 1;

    clilen = sizeof(cli_addr);
    newsockfd = accept(answer_sfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        LOGE(TAG, "ERROR on accept");
        return 1;
    }

    if (setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY, (void *) &on, sizeof(on)) == -1) {
	    LOGE(TAG, "can't set sockopt TCP_NODELAY");
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
			"ATDhostname:port - Dial hostname, port optional (default:" STR_VALUE(DEFAULT_LISTENER_PORT) ")" CRLF,
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

static int _read() {

    unsigned char data;

    if (read(*active_sfd, &data, 1) == 0) {
        /* this will occur if the socket is disconnected */
        LOGI(TAG, "Socket disconnected");
        at_buf[0] = 0;
        at_cat_s(CRLF AT_NO_CARRIER);
        at_cmd[0] = 0;
        at_state = cmd;

        return -1;
    }

    return data;
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

        /**
         * In telnet mode this "clocks" the inbound connection into telnet_recv() 
         * and lets the event handler buffer the input (1 character)
         * then it can check the buffer and report on available data in the buffer
         */

        if ((telnet != NULL) && tn_len) return POLLIN;

        p[0].fd = *active_sfd;
        p[0].events = POLLIN | POLLOUT;
        p[0].revents = 0;
        poll(p, 1, 0);

        if (telnet == NULL) return (p[0].revents & POLLIN);

        if (p[0].revents & POLLIN) {
            int res;
            char data;
            res = _read();
            if (res != -1) {
                data = res;
                telnet_recv(telnet, (char *) &data, 1);
            }
            if (tn_len) return POLLIN;
        } 

        return 0;

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

        if (telnet != NULL) {
            if (tn_len) {
                tn_len = 0;
                return tn_recv;
            } else {
                return -1;
            }
        } else {

            p[0].fd = *active_sfd;
            p[0].events = POLLIN;
            p[0].revents = 0;
            poll(p, 1, 0);

            if (!(p[0].revents & POLLIN)) return -1;

            return (_read());
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
                if (telnet != NULL) {
                    telnet_send(telnet, at_buf, strlen(at_buf));
                } else {
                    write(*active_sfd, at_buf, strlen(at_buf));
                }

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
            if (telnet != NULL) {
                telnet_send(telnet, (char *) &data, 1);
            } else {
                write(*active_sfd, (char *) &data, 1);
            }
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