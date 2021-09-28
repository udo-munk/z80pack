/**
 * netsrv.h
 *
 * Copyright (C) 2018 by David McNaughton
 * 
 * History:
 * 12-JUL-18    1.0     Initial Release
 */

/**
 * This web server module provides...
 */
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define UNUSED(x) (void)(x)

enum net_device {
	DEV_TTY,
	DEV_LPT,
	DEV_VIO,
	DEV_CPA,
	DEV_DZLR,
	DEV_88ACC,
	DEV_D7AIO,
	DEV_PTR,
	_DEV_MAX
};

typedef enum net_device net_device_t;

struct msgbuf {
	long			mtype;
	unsigned char	mtext[128];
};

typedef struct msgbuf msgbuf_t;

struct ws_client {
	struct mg_connection *conn;
	int state;
};

typedef struct ws_client ws_client_t;

extern int net_device_alive(net_device_t);
extern void net_device_send(net_device_t, char*, int);
extern int net_device_get(net_device_t);
extern int net_device_get_data(net_device_t, char *, int);
extern int net_device_poll(net_device_t);
extern void net_device_service(net_device_t device, void (*cbfunc)(BYTE *data));

/*
* convenience macros for http output
*/
typedef struct mg_connection HttpdConnection_t;

#define httpdPrintf(conn, args...)			mg_printf(conn, args)
#define httpdStartResponse(conn, code)		mg_printf(conn, "HTTP/1.1 %d OK\r\n", code)
#define httpdHeader(conn, key, val)			mg_printf(conn, "%s: %s\r\n", key, val)
#define httpdEndHeaders(conn)				mg_printf(conn, "\r\n")

#define _HTTP_MAX_ARGS	8

enum http_method {
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_UNKNOWN
};

typedef enum http_method http_method_t;

struct request {
	const struct mg_request_info *mg;
	http_method_t method;
	const char *args[_HTTP_MAX_ARGS];
	long long len;
};

typedef struct request request_t;

extern request_t *get_request(const HttpdConnection_t *);