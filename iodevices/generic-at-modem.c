/**
 * generic-at-modem.c
 * 
 * Emulation of generic 'AT' modem over TCP/IP sockets (telnet)
 *
 * Copyright (C) 2019-2021 by David McNaughton
 * 
 * History:
 * 12-SEP-19    1.0     Initial Release
 * 29-SEP-19    1.1     Added Answer modes and registers
 * 20-OCT-19    1.2     Added Telnet handler
 * 23-OCT-19    1.3     Put Telnet protocol under modem register control
 * 16-JUL-20	1.4	    fix bug/warning detected with gcc 9
 * 17-JUL-20    1.5     Added/Update AT$ help, ATE, ATQ, AT&A1 cmds, MODEM.init string
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "libtelnet.h"

#define UNUSED(x) (void) (x)

#define LOG_LOCAL_LEVEL LOG_WARN
#include "log.h"

#define MODEM_ID "'AT' Modem"

#define BASE_DECIMAL 10

#define AT_BUF_LEN      81
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
#define SREG_HUP_DELAY 10
#define SREG_OPT    13
#define SREG_PORT   14
#define SREG_TELNET 15
#define SREG_TN_SGA 16
#define SREG_TN_ECHO 17
#define SREG_TN_BIN 18
#define SREG_TN_NAWS 19
#define SREG_COLS   20
#define SREG_ROWS   21
#define SREG_TN_TTYPE 22
#define SREG_FLOW_CTRL 39
#define MAX_REG_NUM 50

#define SREG_DEFAULTS { \
    /* SREG_AA */       0, \
    /* SREG_RINGS */    0, \
    /* SREG_ESCAPE */   43, \
    /* SREG_CR */       13, \
    /* SREG_LF */       10, \
    /* SREG_BS */       8, \
    0, 0, 0, 0, \
    /* SREG_HUP_DELAY */ 14, \
    0, 0, \
    /* SREG_OPT */      1, \
    /* SREG_PORT */     DEFAULT_LISTENER_PORT, \
    /* SREG_TELNET */   0, \
    /* SREG_TN_SGA */   3, \
    /* SREG_TN_ECHO */  3, \
    /* SREG_TN_BIN */   0, \
    /* SREG_TN_NAWS */  3, \
    /* SREG_COLS */     80, \
    /* SREG_ROWS */     24, \
    /* SREG_TN_TTYPE */ 3, \
    0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, \
};

#define OPT_ECHO    0x1
#define OPT_QUIET   0x2

void modem_device_init(void);

static bool daemon_f = false;

static unsigned int s_reg[MAX_REG_NUM] = SREG_DEFAULTS;
static const unsigned int s_reg_defaults[MAX_REG_NUM] = SREG_DEFAULTS;
static int reg = 0;

static int sfd = 0;
static int answer_sfd = 0;
static int newsockfd = 0;

static int *active_sfd = &sfd;

static char addr[AT_BUF_LEN];
static char port_num[11];

static telnet_telopt_t telnet_opts[10];

static int carrier_detect;

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

int time_diff_msec(struct timeval *, struct timeval *);
int time_diff_sec(struct timeval *, struct timeval *);

static void telnet_hdlr(telnet_t *telnet, telnet_event_t *ev, void *user_data) {

    char buf[81];
    char * p;
    int i;

    UNUSED(user_data);

    switch (ev->type) {
    case TELNET_EV_DATA:
		if (ev->data.size) {
            if (ev->data.size != 1) {
                LOGW(TAG, "LONGER THAN EXPECTED [%lld]", (long long) ev->data.size);
            } else {
                tn_recv = *(ev->data.buffer);
                tn_len = ev->data.size;
            }
            LOGD(TAG, "Telnet IN: %c[%lld]", tn_recv, (long long) ev->data.size);
		}
		break;
    case TELNET_EV_SEND:
		if (ev->data.size) {
            p = buf;
            for(i=0;i<(int)ev->data.size;i++) {
                p += sprintf (p, "%d ", *(ev->data.buffer+i));
            }
            LOGD(TAG, "Telnet OUT: %s[%lld]", buf, (long long) ev->data.size);
		}
		write(*active_sfd, ev->data.buffer, ev->data.size);
		break;
    case TELNET_EV_WILL:
        if (ev->neg.telopt == TELNET_TELOPT_SGA) {
            LOGI(TAG, "Telnet WILL SGA");
        } else if (ev->neg.telopt == TELNET_TELOPT_ECHO) {
            LOGI(TAG, "Telnet WILL ECHO");
        } else if (ev->neg.telopt == TELNET_TELOPT_BINARY) {
            LOGI(TAG, "Telnet WILL BINARY");
        } else {
            LOGW(TAG, "Telnet WILL unknown opt: %d", ev->neg.telopt);
        }
		break;
    case TELNET_EV_DO:
        if (ev->neg.telopt == TELNET_TELOPT_SGA) {
            LOGI(TAG, "Telnet DO SGA");
        } else if (ev->neg.telopt == TELNET_TELOPT_ECHO) {
            LOGI(TAG, "Telnet DO ECHO");
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
        const char *ttype;
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
    carrier_detect = 0;
}

int open_socket(void) {

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    void *addrptr = NULL;
    uint16_t port = 0;
    struct sockaddr_in *sktv4 = NULL;
    /* struct sockaddr_in6 *sktv6 = NULL; */
    int on = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow only IPv4 not IPv6 */
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(addr, port_num, &hints, &result);
    if (s != 0) {
        LOGI(TAG, "getaddrinfo: %s", "failed");
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
                /*
                sktv6 = (struct sockaddr_in6 *)rp->ai_addr;
                port = sktv6->sin6_port;
                addrptr = &sktv6->sin6_addr;
                */
                LOGE(TAG, "Not expecting IPV6 addresses");
                return 1;
            default:
                LOGE(TAG, "Not expecting address family type %d", rp->ai_family);
                return 1;
        }

        inet_ntop(rp->ai_family, addrptr, addr, 100);
        LOGI(TAG, "Address: %s:%d", addr, ntohs(port));

        if ((sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0) {
            LOGE(TAG, "Failed to create socket");
            return 1;
        };

        active_sfd = &sfd;
        carrier_detect = 1;
        LOGI(TAG, "Socket created");

        if (connect(sfd, rp->ai_addr, sizeof(struct sockaddr_in)) < 0) { 
            LOGW(TAG, "Failed to connect to socket: %d", errno);
            close_socket();
            return 1;
        }
        LOGI(TAG, "Socket connected");

        if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *) &on, sizeof(on)) == -1) {
	        LOGE(TAG, "can't set sockopt TCP_NODELAY");
            close_socket();
	        return 1;
	    }

        /* Initialize Telnet session */
        if (s_reg[SREG_TELNET]) {
            init_telnet_opts();
            if ((telnet = telnet_init(telnet_opts, telnet_hdlr, 0, NULL)) == 0) {
                LOGE(TAG, "can't initialize telnet session");
                close_socket();
                return 1;
            } else {
                LOGI(TAG, "Telnet session started");
            };
        };

        return 0;
    }

    return 1;
}

int hangup_timeout(bool start) {
    static struct timeval hup_t1, hup_t2;
    static int waiting = 0;
    int tdiff;

    if (*active_sfd) {

         if (start) { 
            gettimeofday(&hup_t1, NULL);
            waiting = 1;
            LOGI(TAG, "Waiting to HUP");
        } else if (waiting) {
            gettimeofday(&hup_t2, NULL);
            tdiff = time_diff_msec(&hup_t1, &hup_t2) / 100; /* scale msec to 10ths of seconds */
            if (tdiff >=  (int)(s_reg[SREG_HUP_DELAY])) { /* SREG_HUP_DELAY is in 10ths of seconds */
                waiting = 0;
                s_reg[SREG_RINGS] = 0;
                close_socket();
                LOGI(TAG, "HUP timeout - closing socket");
			    return 1;
            }
        }
    } else {
        waiting = 0;
    }
    return 0;
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

    inet_ntop(AF_INET, &cli_addr.sin_addr, addr, 100);
    LOGI(TAG, "New Remote Connection: %s:%d", addr, ntohs(cli_addr.sin_port));
    active_sfd = &newsockfd;
    carrier_detect = 1;

    if (setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY, (void *) &on, sizeof(on)) == -1) {
	    LOGE(TAG, "can't set sockopt TCP_NODELAY");
        close_socket();
    	return 1;
    }

    /* Initialize Telnet session */
    if (s_reg[SREG_TELNET]) {
        init_telnet_opts();
        if ((telnet = telnet_init(telnet_opts, telnet_hdlr, 0, NULL)) == 0) {
            LOGE(TAG, "can't initialize telnet server session");
            close_socket();
            return 1;
        } else {
            LOGI(TAG, "Telnet server session started");
        };
        telnet_negotiate(telnet, (s_reg[SREG_TN_SGA] & 1) ? TELNET_WILL : TELNET_WONT , TELNET_TELOPT_SGA);
	    telnet_negotiate(telnet, (s_reg[SREG_TN_ECHO] & 1) ? TELNET_WILL : TELNET_WONT, TELNET_TELOPT_ECHO);
    };

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

char at_buf[AT_BUF_LEN * 2] = "";
char at_cmd[AT_BUF_LEN] = "";
char at_prev[AT_BUF_LEN] = "";
char at_err[AT_BUF_LEN * 2] = "";

char *at_out = at_buf;
enum at_states { cmd, A_recv, AT_recv, AS_recv, dat, intr, help };
typedef enum at_states at_state_t;

at_state_t at_state = cmd;

#define CR			"\r"
#define LF			"\n"
#define CRLF		CR LF
#define END		    NULL

#define AT_OK		    "OK" CRLF
#define AT_ERROR	    "ERROR" CRLF
#define AT_NO_ANSWER	"NO ANSWER" CRLF
#define AT_NO_CARRIER	"NO CARRIER" CRLF
#define AT_NO_DIALTONE	"NO DIALTONE" CRLF
#define AT_RING	        "RING" CRLF

static const char *at_help[] = {
			LF "AT COMMANDS:" CRLF,
			"AT  - 'AT' Test                 ",
			"| A/  - (immediate) Repeat last" CRLF,
			"AT$ - Help                      ",
			"| ATIn - Information " CRLF,
			"ATZ - Reset modem" CRLF,
			"AT&F - Restore factory defaults ",
#ifdef MODEM_NVRAM
			"| AT&W - Write settings to NVRAM" CRLF,
#else
            CRLF,
#endif
			"ATDhostname:port - Dial hostname, port optional (default:" STR_VALUE(DEFAULT_LISTENER_PORT) ")" CRLF,
			"+++ - Return to command mode    ",
			"| ATO - Return Online" CRLF,
			"ATH - Hangup" CRLF,
			"AT&An - Enable Answer mode - 0=interactive, 1=daemon" CRLF,
			"ATA - Answer" CRLF,
			"ATSn - Select register n" CRLF,
			"AT? - Query current register    ",
			"| AT=r - Set current register to r" CRLF,
			"ATEn - Command Echo 0=off, 1=on ",
			"| ATQn - Quiet Results 0=off, 1=on" CRLF,
#ifdef MODEM_UART
			"AT&K0 - Disable flow control    ",
			"| AT&K1 - Enable RTS/CTS flow control" CRLF,
#endif
#ifdef MODEM_WIFI
			"AT+W? - Query WiFi AP Join status" CRLF,
			"AT+W=ssid,password - Join WiFI AP" CRLF,
			"AT+W$ - Show WiFi IP address   ",
			"| AT+W# - Show WiFi MAC address" CRLF,
			"AT+W+ - Reconnect WiFI AP       ",
			"| AT+W- - Quit WiFi AP" CRLF,
			"AT+U? - Query OTA Update        ",
			"| AT+U=url - Set custom URL for OTA update" CRLF,
			"AT+U^ - Upgrade to OTA Update   ",
			"| AT+U! - Force Upgrade to OTA Update" CRLF,
			"AT+U$ - Show OTA Partition Status" CRLF,
#endif
#ifdef MODEM_UART
			"AT+B? - Query Baud Rate         ",
			"| AT+B=nnn - Set Baud Rate (4800..115200)" CRLF,
#endif
			"AT+T? - Query Telnet TERM       ",
			"| AT+T=name - Set Telnet TERM" CRLF,
            END
};

static const char **msg;

void at_cat_c(char c) {
    if (strlen(at_cmd) >= AT_BUF_LEN - 1) {
        LOGE(TAG, "Buffer overflow");
        return;
    }

	if (c == '\b') { /*TODO: use s_reg[SREG_BS] */
		if(strlen(at_cmd) > 2) {
            if (s_reg[SREG_OPT] & OPT_ECHO) {
                at_buf[strlen(at_buf) + 3] = 0;
                at_buf[strlen(at_buf) + 2] = c;
                at_buf[strlen(at_buf) + 1] = ' ';
                at_buf[strlen(at_buf)] = c;
            }
			at_cmd[strlen(at_cmd) - 1] = 0;
		}
	} else {
        if (s_reg[SREG_OPT] & OPT_ECHO) {
            at_buf[strlen(at_buf) + 1] = 0;
            at_buf[strlen(at_buf)] = c;
        }
		at_cmd[strlen(at_cmd) + 1] = 0;
		at_cmd[strlen(at_cmd)] = c;
	}
	LOGD(TAG, "AT CMD: [%s]", at_cmd);
}

void at_cat_s(const char *s) {
    if (s_reg[SREG_OPT] & OPT_QUIET) return;
    if ((strlen(at_buf) + strlen(s)) >= (AT_BUF_LEN*2) - 1) {
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

#define AT_END_CMD	        (at_ptr = &at_cmd[strlen(at_cmd)])
#define AT_NEXT_CMD 	    (at_ptr++)
#define AT_NEXT_CMD2        (at_ptr += 2)
#define AT_NEXT_CMD_ARGS    (at_ptr = arg_ptr)

  while (*at_ptr != 0) {
    LOGD(__func__, "AT CMD: [%s]", at_ptr);
	switch (*at_ptr++) {
        /*TODO: add a command to check connection status */
		case '\r': /* AT<CR> */ /*TODO: use s_reg[SREG_CR] */
			break;
        case 'I':
            tmp_reg = strtol(at_ptr, &arg_ptr, BASE_DECIMAL);

            if (tmp_reg == 0) {
                strcpy(at_err, LF MODEM_ID CRLF);
                at_cat_s(at_err);
#ifdef MODEM_ESP32
            } else if (tmp_reg == 1) {
                const esp_app_desc_t* desc = esp_ota_get_app_description();
                sprintf(at_err, LF "%s" CRLF, desc->version);
                at_cat_s(at_err);
            } else if (tmp_reg == 2) {
                sprintf(at_err, LF "%s" CRLF, esp_get_idf_version());
                at_cat_s(at_err);
            } else if (tmp_reg == 3) {
                const esp_app_desc_t* desc = esp_ota_get_app_description();
                sprintf(at_err, LF "%s" CRLF, desc->project_name);
                at_cat_s(at_err);
            } else if (tmp_reg == 4) {
                sprintf(at_err, LF "DMA %d" CRLF, heap_caps_get_free_size(MALLOC_CAP_DMA));
                at_cat_s(at_err);
            } else if (tmp_reg == 5) {
                sprintf(at_err, LF "INTERNAL %d" CRLF, heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
                at_cat_s(at_err);
            } else if (tmp_reg == 6) {
                sprintf(at_err, LF "TOTAL %d" CRLF, heap_caps_get_free_size(MALLOC_CAP_32BIT));
                at_cat_s(at_err);
#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
            } else if (tmp_reg == 8) {
                char buff[1024];
                vTaskGetRunTimeStats(buff);
                fwrite(buff, strlen(buff), 1, stdout); /* Writes to FTDI UART not the modem */
                at_cat_s(LF "TASK TIMES" CRLF);
#endif
#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
            } else if (tmp_reg == 9) {
                char buff[1024];
                vTaskList(buff);
                fwrite(buff, strlen(buff), 1, stdout); /* Writes to FTDI UART not the modem */
                at_cat_s(LF "TASKS" CRLF);
#endif
#endif
            } else {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }
            AT_NEXT_CMD_ARGS;
            break;
		case '$': /* AT$ */
            at_state = help;
            msg = at_help;
			break;
		case 'Z': /* ATZ */
            memset(at_prev, 0, sizeof(at_prev));
            modem_device_init();
			AT_END_CMD;
            break;
		case 'H': /* ATH */
            if (*active_sfd) {
                AT_END_CMD;
                s_reg[SREG_RINGS] = 0;
                close_socket();
			    at_cat_s(LF "HANGUP" CR);
            }
			break;
		case 'D': /* ATDaddr[:port=23] */
#ifdef MODEM_WIFI
            if (WiFi_status() != 3) {
                strcpy(at_err, LF AT_NO_DIALTONE CRLF);
			    return 1;
            }
#endif
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
                strcpy(port_num, STR_VALUE(DEFAULT_LISTENER_PORT));
            }
            LOGI(TAG, "Port: [%s]", port_num);

            if (open_socket()) {
                strcpy(at_err, LF AT_NO_ANSWER CRLF);
                return 1;
            };
			AT_END_CMD;
            at_state = dat;
            if (telnet) {
                at_cat_s(LF "CONNECT TELNET" CRLF);
            } else {
                at_cat_s(LF "CONNECT" CRLF);
            }
            *at_err = 0;
            return 1; /* Not an error, just want to suppress the OK message */
		case 'S': /* ATSn */
            tmp_reg = strtol(at_ptr, &arg_ptr, BASE_DECIMAL);

            if (tmp_reg < 0 || tmp_reg > (MAX_REG_NUM - 1) || arg_ptr == at_ptr) {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }

            reg = tmp_reg;
            LOGI(TAG, "AT Register set to %d", reg);
            AT_NEXT_CMD_ARGS;
			break;
		case '?': /* AT? */
            LOGI(TAG, "ATS%d? is %d", reg, s_reg[reg]);
            sprintf(at_err, LF "%d" CRLF, s_reg[reg]);
            at_cat_s(at_err);
			break;
		case '=': /* AT=r */
            tmp_reg = strtol(at_ptr, &arg_ptr, BASE_DECIMAL);

            if (tmp_reg < 0 || tmp_reg > 255 || arg_ptr == at_ptr) {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }

            LOGI(TAG, "ATS%d = %d", reg, tmp_reg);
            AT_NEXT_CMD_ARGS;
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
        case 'E': /* ATE - Command echo */
            tmp_reg = strtol(at_ptr, &arg_ptr, BASE_DECIMAL);
            AT_NEXT_CMD_ARGS;
            if (tmp_reg < 0 || tmp_reg > 1) {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }
            if (tmp_reg) { 
                s_reg[SREG_OPT] |= OPT_ECHO;
            } else {
                s_reg[SREG_OPT] &= ~OPT_ECHO;
            }
            break;
        case 'Q': /* ATQ - Quiet mode */
            tmp_reg = strtol(at_ptr, &arg_ptr, BASE_DECIMAL);
            AT_NEXT_CMD_ARGS;
            if (tmp_reg < 0 || tmp_reg > 1) {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }
            if (tmp_reg) { 
                s_reg[SREG_OPT] |= OPT_QUIET;
            } else {
                s_reg[SREG_OPT] &= ~OPT_QUIET;
            }
            break;
        case '&': 
            if (*at_ptr == 'A') { /* AT&A - enable answer - listen */
                tmp_reg = strtol(++at_ptr, &arg_ptr, BASE_DECIMAL);
                AT_NEXT_CMD_ARGS;
#ifdef MODEM_WIFI
                if (WiFi_status() != 3) {
                    strcpy(at_err, LF AT_NO_DIALTONE CRLF);
                    return 1;
                }
#endif
                if (answer_init()) {
                    strcpy(at_err, LF AT_ERROR CRLF);
                    return 1;
                };
                if (tmp_reg) {
                    daemon_f = true;
                    s_reg[SREG_OPT] &= ~OPT_ECHO;
                    s_reg[SREG_OPT] |= OPT_QUIET;
                    LOGI(TAG, "MODEM will be daemon and listen (No Echo & Quiet)");
                }
			    break;
            } else if (*at_ptr == 'F') { /* AT&F - reset to Factory defaults */
                for (tmp_reg = 0; tmp_reg < MAX_REG_NUM; tmp_reg++) {
                    s_reg[tmp_reg] = s_reg_defaults[tmp_reg];
                }
                LOGI(TAG, "Reset to Factory defaults");
                AT_NEXT_CMD;
			    break;
#ifdef MODEM_NVRAM
            } else if (*at_ptr == 'W') { /* AT&W - write settings to NVRAM */
                Modem_nvramWriteAllSreg(s_reg, MAX_REG_NUM);
                LOGI(TAG, "Settings written to NVRAM");
                AT_NEXT_CMD;
			    break;
#endif
#ifdef MODEM_UART
            } else if (*at_ptr == 'K') {
                tmp_reg = strtol(++at_ptr, &arg_ptr, BASE_DECIMAL);
                AT_NEXT_CMD_ARGS;
                if (tmp_reg == 0) { /* AT&K0 - disable flow control */
                    s_reg[SREG_FLOW_CTRL] = 0;
                    /* clear UART flow control */
                    Modem_setFlowControl(0);
                    LOGI(TAG, "Flow Control Disabled");
                } else if (tmp_reg == 1) { /* AT&K1 - enable RTS/CTS flow control */
                    s_reg[SREG_FLOW_CTRL] = 1;
                    /* set UART RTS/CTS flow control */
                    Modem_setFlowControl(1);
                    LOGI(TAG, "RTS/CTS Flow Control Enabled");
                } else {
                    strcpy(at_err, LF AT_ERROR CRLF);
                    return 1;
                }
#endif
            } else {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }
            break;
        case '+': 
            if (*at_ptr == 'T' && *(at_ptr+1) == '?') {
                const char *ttype;
                if ((ttype = getenv("TERM")) == NULL) {
                    ttype = TELNET_TTYPE;
                }
                sprintf(at_err, LF "%s" CRLF, ttype);
                at_cat_s(at_err);
                AT_NEXT_CMD2;
            } else if (*at_ptr == 'T' && *(at_ptr+1) == '=') {
                strcpy(at_err, at_ptr+2);
                arg_ptr = strtok(at_err, "\r");

                if(arg_ptr == NULL) {
                    unsetenv("TERM");
                    LOGI(TAG, "TERM cleared");
                } else {
                    setenv("TERM", arg_ptr, 1);
                    LOGI(TAG, "TERM: [%s]", getenv("TERM"));
                }
#ifdef MODEM_NVRAM
                Modem_nvramWriteEnv("TERM");
#endif
                AT_END_CMD;
#ifdef MODEM_WIFI
/**
 *  WIFI CONFIG HANDLING
 */ 
            } else if (*at_ptr == 'W' && *(at_ptr+1) == '=') {

                char ssid[MAX_SSID + 1] = "";
                char psw[MAX_PWD + 1] = "";

                strcpy(at_err, at_ptr+2);
                arg_ptr = strtok(at_err, ",\r");

                if(arg_ptr == NULL) {
                    strcpy(at_err, LF AT_ERROR CRLF);
                    return 1;
                }
                strncpy(ssid, arg_ptr, MAX_SSID);

                arg_ptr =  strtok(NULL, "\r");
                if (arg_ptr != NULL) {
                    strncpy(psw, arg_ptr, MAX_PWD);
                }

                WiFi_begin(ssid, psw);
                AT_END_CMD;
            } else if (*at_ptr == 'W' && *(at_ptr+1) == '?') {
                sprintf(at_err, LF "%s" CRLF, WiFi_status_str());
                at_cat_s(at_err);
                AT_NEXT_CMD2;
            } else if (*at_ptr == 'W' && *(at_ptr+1) == '+') {
                if (WiFi_status() == 255) {
                    WiFi_begin_x();
                }
                sprintf(at_err, LF "%s" CRLF, WiFi_reconnect());
                at_cat_s(at_err);
                AT_NEXT_CMD2;
            } else if (*at_ptr == 'W' && *(at_ptr+1) == '-') {
                sprintf(at_err, LF "%s" CRLF, WiFi_disconnect());
                at_cat_s(at_err);
                AT_NEXT_CMD2;
            } else if (*at_ptr == 'W' && *(at_ptr+1) == '$') {
                sprintf(at_err, LF "%s" CRLF, WiFi_IP());
                at_cat_s(at_err);
                AT_NEXT_CMD2;
            } else if (*at_ptr == 'W' && *(at_ptr+1) == '#') {
                sprintf(at_err, LF "%s" CRLF, WiFi_MAC());
                at_cat_s(at_err);
                AT_NEXT_CMD2;
            } else if (*at_ptr == 'U' && *(at_ptr+1) == '=') {

                if (WiFi_status() != 3) {
                    strcpy(at_err, LF AT_NO_DIALTONE CRLF);
                    return 1;
                }

                strcpy(at_err, at_ptr+2);
                arg_ptr = strtok(at_err, "\r");

                if(arg_ptr == NULL) {
                    strcpy(at_err, LF AT_ERROR CRLF);
                    return 1;
                }

                if (ota_setURL(at_err, arg_ptr)) {
                    return 1;
                }

                at_cat_s(at_err);
                AT_END_CMD;
            } else if (*at_ptr == 'U' && *(at_ptr+1) == '?') {

                if (WiFi_status() != 3) {
                    strcpy(at_err, LF AT_NO_DIALTONE CRLF);
                    return 1;
                }

                at_err[0] = *(at_ptr+2); 

                if (ota_start(at_err)) {
                    return 1;
                }

                at_cat_s(at_err);
                AT_NEXT_CMD2;
            } else if (*at_ptr == 'U' && *(at_ptr+1) == '$') {

                if (ota_partitionQuery(at_err)) {
                    return 1;
                }

                at_cat_s(at_err);
                AT_NEXT_CMD2;
            } else if (*at_ptr == 'U' && *(at_ptr+1) == '^') {

                if (ota_upgrade(at_err)) {
                    return 1;
                }

                at_cat_s(at_err);
                AT_NEXT_CMD2;
            } else if (*at_ptr == 'U' && *(at_ptr+1) == '!') {

                if (ota_forceUpgrade(at_err)) {
                    return 1;
                }

                at_cat_s(at_err);
                AT_NEXT_CMD2;
#endif
#ifdef MODEM_UART
/**
 *  SERIAL CONFIG HANDLING
 */ 
            } else if (*at_ptr == 'B' && *(at_ptr+1) == '=') {
                tmp_reg = strtol(at_ptr+2, &arg_ptr, BASE_DECIMAL);

                if (tmp_reg < 4800 || tmp_reg > 115200 || arg_ptr == (at_ptr+2)) {
                    strcpy(at_err, LF AT_ERROR CRLF);
                    return 1;
                }

                Modem_updateBaudRate(tmp_reg);
                sprintf(at_err, LF "%d" CRLF, Modem_baudRate());
                at_cat_s(at_err);
                AT_NEXT_CMD_ARGS;
            } else if (*at_ptr == 'B' && *(at_ptr+1) == '?') {
                sprintf(at_err, LF "%d" CRLF, Modem_baudRate());
                at_cat_s(at_err);
                AT_NEXT_CMD2;
#endif
/**
 * 
 */
            } else {
                strcpy(at_err, LF AT_ERROR CRLF);
                return 1;
            }
            break;
        case 'A': /* ATA - answer */
#ifdef MODEM_WIFI
            if (WiFi_status() != 3) {
                strcpy(at_err, LF AT_NO_DIALTONE CRLF);
			    return 1;
            }
#endif
            answer();
            AT_END_CMD;
            at_state = dat;
            break;
		default:
			strcpy(at_err, LF AT_ERROR CRLF);
			return 1;
	}
  }

	return 0;
}

int time_diff_msec(struct timeval *t1, struct timeval *t2)
{
	long sec, usec;

	sec = (long) t2->tv_sec - (long) t1->tv_sec;
	usec = (long) t2->tv_usec - (long) t1->tv_usec;
	/* normalize result */
	if (usec < 0L) {
		sec--;
		usec += 1000000L;
	}
    return (sec * 1000) + (usec / 1000);
}

int time_diff_sec(struct timeval *t1, struct timeval *t2)
{
	long sec, usec;

	sec = (long) t2->tv_sec - (long) t1->tv_sec;
	usec = (long) t2->tv_usec - (long) t1->tv_usec;
	/* normalize result */
	if (usec < 0L) {
		sec--;
		/* usec += 1000000L; */
	}
    return sec;
}

static struct timeval at_t1, at_t2;
int tdiff;

int modem_device_poll(int i);

int modem_device_alive(int i) {

    UNUSED(i);

    if (!daemon_f) return 1;

    if (answer_sfd) {
        if (*active_sfd) {
            return 1;
        } else {
            if (at_state == cmd) {
                modem_device_poll(0);
                return 0;
            }
        }
    } 
    return 0;
}

static int _read(void) {

    unsigned char data;

    if (read(*active_sfd, &data, 1) == 0) {
        /* this will occur if the socket is disconnected */
        LOGI(TAG, "Socket disconnected");
        at_buf[0] = 0;
        at_cat_s(CRLF AT_NO_CARRIER);
        hangup_timeout(true);
        at_cmd[0] = 0;
        at_state = cmd;
        carrier_detect = 0;

        return -1;
    }

    return data;
}

int modem_device_poll(int i) {

    UNUSED(i);

    struct pollfd p[1];

    if (at_state == help) {
        if (strlen(at_buf) == 0) {
            at_cat_s(*msg);
            msg++;
            if (*msg == NULL) {
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
    } else if (at_state == dat  && strlen(at_out) == 0) {

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
            at_cat_s(CRLF AT_RING);
            if ((s_reg[SREG_AA] > 0) && (s_reg[SREG_RINGS] >= s_reg[SREG_AA])) {
                if (!answer()) {
                    at_state = dat;
                } 
            }
        } else if(hangup_timeout(false)) {
            at_cat_s(CRLF "HANGUP" CRLF);
        }

        return (strlen(at_out) > 0);
    }
}

int modem_device_get(int i) {

    UNUSED(i);

    struct pollfd p[1];
    unsigned char data;

    if (at_state == dat && strlen(at_out) == 0) {

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

    UNUSED(i);

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
        /* fall through */
        case dat:
        	gettimeofday(&at_t2, NULL);
            tdiff = time_diff_sec(&at_t1, &at_t2);

            if (data == '+' && (tdiff > 0) && !daemon_f) {
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

int modem_device_carrier(int i) {
    UNUSED(i);
    
    return carrier_detect;
}

void modem_device_init(void) {
    if (*active_sfd) {
        close_socket();
    }
    if (answer_sfd) {
        active_sfd = &answer_sfd;
        close_socket();
        active_sfd = &sfd;

    }
    at_state = cmd;

#ifdef MODEM_NVRAM
    int tmp_reg;
    for (tmp_reg = 0; tmp_reg < MAX_REG_NUM; tmp_reg++) {
        s_reg[tmp_reg] = Modem_nvramReadSreg(tmp_reg, s_reg_defaults[tmp_reg]);
    }
    Modem_nvramReadEnv("TERM");
    Modem_nvramReadEnv("MODEM.init");
#else
    memcpy(s_reg, s_reg_defaults, sizeof(s_reg_defaults));
#endif
    char *modem_init_string;
    if ((modem_init_string = getenv("MODEM.init")) != NULL) {
        LOG(TAG, "MODEM.init string: %s\r\n", modem_init_string);
        while (*modem_init_string) {
            modem_device_send(0, *modem_init_string++);
        }
        modem_device_send(0, '\r');
        while (modem_device_poll(0)) {
            modem_device_get(0);
        }
    }
#ifdef MODEM_UART
    /* set/clear UART flow control based on S39 */
    Modem_setFlowControl(s_reg[SREG_FLOW_CTRL]);
#endif
}
