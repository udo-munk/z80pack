/**
 * netsrv.c
 *
 * Copyright (C) 2018 by David McNaughton
 * 
 * History:
 * 12-JUL-18    1.0     Initial Release
 */

/**
 * This web server module provides...
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include "sim.h"
#include "simglb.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"
#include "log.h"
#include "netsrv.h"
#include "civetweb.h"

#ifdef HAS_NETSERVER

#define DOCUMENT_ROOT "./www"
#define PORT "8080"

#define MAX_WS_CLIENTS (5)
static const char *TAG = "netsrv";

static int queue[MAX_WS_CLIENTS];

static msgbuf_t msg;

static ws_client_t ws_clients[MAX_WS_CLIENTS];

char *dev_name[] = {
	"SIO1",
	"LPT",
	"VIO",
	"CPA"
};

int last_error = 0; //TODO: replace
/*
extern int reset;
extern int power;
extern void quit_callback(void);
*/

#ifdef HAS_DISKMANAGER
extern int LibraryHandler(struct mg_connection *, void *);
extern int DiskHandler(struct mg_connection *, void *);
#endif

/**
 * Check if a queue is provisioned
 */
int net_device_alive(net_device_t device) {
	return queue[device];
}

/**
 * Assumes the data is:
 * 		TEXT	if only a single byte
 * 		BINARY	if there are multiple bytes
 */
void net_device_send(net_device_t device, char* msg, int len) {
 	if(queue[device]) {
		mg_websocket_write(ws_clients[device].conn,
			(len==1)?MG_WEBSOCKET_OPCODE_TEXT:MG_WEBSOCKET_OPCODE_BINARY,
			msg, len);
	}
}

/**
 * Always removes something from the queue is data waiting
 * returns:
 * 		char	if data is waiting in the queue
 * 		-1		if the queue is not open, is empty
 */
int net_device_get(net_device_t device) {
	ssize_t res;
	msgbuf_t msg;

	if (queue[device]) {
		res = msgrcv(queue[device], &msg, 2, 1L, IPC_NOWAIT);
		LOGD(TAG, "GET: device[%d] res[%ld] msg[%ld, %s]\r\n", device, res, msg.mtype, msg.mtext);
		if (res == 2) {
			return msg.mtext[0];
		}
	}

	return -1;
}

/**
 * Doesn't remove data from the queue
 * returns:
 * 		1	if data is waiting in the queue
 * 		0	if the queue is not open or is empty
 */
int net_device_poll(net_device_t device) {
	ssize_t res;
	msgbuf_t msg;

	if (queue[device]) {
		res = msgrcv(queue[device], &msg, 1, 1L, IPC_NOWAIT);
		LOGV(TAG, "POLL: device[%d] res[%ld] errno[%d]", device, res, errno);
		if (res == -1 && errno == E2BIG) {
			LOGD(TAG, "CHARACTERS WAITING");
			return 1;
		}
	}
	return 0;
}

request_t *get_request(const HttpdConnection_t *conn) {
	static request_t req;

	req.mg = mg_get_request_info(conn);

    if (!strcmp(req.mg->request_method, "GET")) {
		req.method = HTTP_GET;
    } else if (!strcmp(req.mg->request_method, "POST")) {
		req.method = HTTP_POST;
    } else if (!strcmp(req.mg->request_method, "PUT")) {
		req.method = HTTP_PUT;
    } else if (!strcmp(req.mg->request_method, "DELETE")) {
		req.method = HTTP_DELETE;
    } else {
		req.method = UNKNOWN;
	}

	//TODO: split query_string on '&' into args[] - for now its all jammed into args[0]
	req.args[0] = req.mg->query_string;
	req.len = req.mg->content_length;

	return &req;
}

int log_message(const HttpdConnection_t *conn, const char *message)
{
	UNUSED(conn);

	puts(message);
	return 1;
}

void InformWebsockets(struct mg_context *ctx)
{
	static unsigned long cnt = 0;
	char text[32];
	int i;

	UNUSED(cnt);

	// sprintf(text, "%lu", ++cnt);
	sprintf(text, "%c", 0);

	mg_lock_context(ctx);
	for (i = 0; i < MAX_WS_CLIENTS; i++) {
		if (ws_clients[i].state == 2) {
			mg_websocket_write(ws_clients[i].conn,
			                   MG_WEBSOCKET_OPCODE_TEXT,
			                   text,
			                   strlen(text));
		}
	}
	mg_unlock_context(ctx);
}

int SystemHandler(HttpdConnection_t *conn, void *unused) {
    request_t *req = get_request(conn);
	UNUSED(unused);

    switch(req->method) {
    case HTTP_GET:
		LOGD(TAG, "Sending SYS: details.");
        httpdStartResponse(conn, 200); 
        httpdHeader(conn, "Content-Type", "application/json");
        httpdEndHeaders(conn);

        httpdPrintf(conn, "{");
                 
            httpdPrintf(conn, "\"system\": { ");
                httpdPrintf(conn, "\"free_mem\": %d, ", 0);
                httpdPrintf(conn, "\"time\": %ld, ", time(NULL));
                httpdPrintf(conn, "\"uptime\": %d ", 0);
            httpdPrintf(conn, "}, ");

            httpdPrintf(conn, "\"state\": { ");
                httpdPrintf(conn, "\"last_error\": %d, ", last_error);
                httpdPrintf(conn, "\"cpu_error\": %d, ", cpu_error);
                httpdPrintf(conn, "\"reset\": %d, ", 0 /*reset*/);
                httpdPrintf(conn, "\"power\": %d ", 0 /*power*/);
            httpdPrintf(conn, "}, ");

            httpdPrintf(conn, "\"about\": { ");
                httpdPrintf(conn, "\"%s\": \"%s\", ", "USR_COM", USR_COM);
                httpdPrintf(conn, "\"%s\": \"%s\", ", "USR_REL", USR_REL);
                httpdPrintf(conn, "\"%s\": \"%s\", ", "USR_CPR", USR_CPR);
                httpdPrintf(conn, "\"%s\": \"%s\", ", "cpu", cpu==Z80?"Z80":"I8080");
                if(x_flag) {
                    httpdPrintf(conn, "\"%s\": \"%s\", ", "bootrom", xfn);
                }
                // if(cpa_attached) {
                //     httpdPrintf(conn, "\"%s\": %d, ", "cpa", dip_settings);
                // }
                httpdPrintf(conn, "\"%s\": %d ", "clock", f_flag);
            httpdPrintf(conn, "} ");

        httpdPrintf(conn, "}");
		break;
    case HTTP_DELETE:
        httpdStartResponse(conn, 205);
		httpdEndHeaders(conn);
		//TODO: make this a bit smarter
		// quit_callback();
		break;
	default:
		httpdStartResponse(conn, 405);  //http error code 'Method Not Allowed'
        httpdEndHeaders(conn);
        break;
	}

	return 1;
}

int DirectoryHandler(HttpdConnection_t *conn, void *path) {
    request_t *req = get_request(conn);
    struct dirent *pDirent;
    DIR *pDir;
	int i = 0;

    switch(req->method) {
    case HTTP_GET:
        pDir = opendir ((char *)path);
        if (pDir == NULL) {
            httpdStartResponse(conn, 404);  //http error code 'Not Found'
            httpdEndHeaders(conn);
        } else {
            httpdStartResponse(conn, 200); 
            httpdHeader(conn, "Content-Type", "application/json");
            httpdEndHeaders(conn);
    
            httpdPrintf(conn, "[");

            while ((pDirent = readdir(pDir)) != NULL) {
                LOGD(TAG, "GET directory: %s type: %d", pDirent->d_name, pDirent->d_type);
                if (pDirent->d_type==DT_REG) {
					httpdPrintf(conn, "%c\"%s\"", (i++ > 0)?',':' ', pDirent->d_name);
                 }
            }
            closedir (pDir);
            httpdPrintf(conn, "]");
        }
		break;
	default:
        httpdStartResponse(conn, 405);  //http error code 'Method Not Allowed'
        httpdEndHeaders(conn);
		break;
    }
	return 1;
}

int UploadHandler(HttpdConnection_t *conn, void *path) {
    request_t *req = get_request(conn);
	int filelen;
	char output[MAX_LFN];

    switch (req->method) {
    case HTTP_PUT:
		strncpy(output, path, MAX_LFN);

		if (output[strlen(output)-1] != '/')
			strncat(output, "/", MAX_LFN - strlen(output));

		strncat(output, req->args[0], MAX_LFN - strlen(output));

		filelen = 0;
		filelen = mg_store_body(conn, output);

        LOGI(TAG, "%d bytes written to %s, received %d", filelen, output, (int) req->len);
        httpdStartResponse(conn, 200); 
        httpdHeader(conn, "Content-Type", "application/json");
        httpdEndHeaders(conn);

        httpdPrintf(conn, "{");
        httpdPrintf(conn, "\"filename\": \"%s\", ", output);
        httpdPrintf(conn, "\"size\": \"%d\" ", filelen);
        httpdPrintf(conn, "}");
		break;
	default:
        httpdStartResponse(conn, 405);  //http error code 'Method Not Allowed'
        httpdEndHeaders(conn);
		break;
    }
	return 1;
}

int ConfigHandler(HttpdConnection_t *conn, void *path) {
    request_t *req = get_request(conn);

    switch (req->method) {
    case HTTP_GET:
        DirectoryHandler(conn, path);
		break;
    case HTTP_PUT:
	    UploadHandler(conn, path);
		break;
	default:
        httpdStartResponse(conn, 405);  //http error code 'Method Not Allowed'
        httpdEndHeaders(conn);
		break;
    }
	return 1;
}

int WebSocketConnectHandler(const HttpdConnection_t *conn, void *device) {
	struct mg_context *ctx = mg_get_context(conn);
	int reject = 1;
    int res;

	mg_lock_context(ctx);
		if (ws_clients[(net_device_t)device].conn == NULL) {
			ws_clients[(net_device_t)device].conn = (struct mg_connection *)conn;
			ws_clients[(net_device_t)device].state = 1;
			mg_set_user_connection_data(ws_clients[(net_device_t)device].conn, (void *)(ws_clients + (net_device_t)device));

			switch ((net_device_t)device) {
			case DEV_SIO1:
			case DEV_VIO:
			case DEV_LPT:
				res = msgget(IPC_PRIVATE, 0644 | IPC_CREAT); //TODO: check flags
				if (res > 0) {
					queue[(net_device_t)device] = res;
				} else {
					perror("msgget()");
				}
				break;
			default:
				break;
			}
			reject = 0;
		}
	mg_unlock_context(ctx);

	LOGD(TAG, "Websocket client %s", (reject ? "rejected" : "accepted"));

	return reject;
}

void WebSocketReadyHandler(HttpdConnection_t *conn, void *device) {
	const char *text = "\r\nConnected to the OSX port of Z80PACK\r\n";
	ws_client_t *client = mg_get_user_connection_data(conn);

	if ((net_device_t) device == DEV_SIO1) 
		mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, text, strlen(text));
	
	if ((net_device_t) device == DEV_VIO) {
		BYTE mode = dma_read(0xf7ff);
		dma_write(0xf7ff, 0x00);
		SLEEP_MS(100);
		dma_write(0xf7ff, mode);
	}

	LOGI(TAG, "WS CLIENT CONNECTED to %s", dev_name[(net_device_t) device]);

	client->state = 2;
}

int WebsocketDataHandler(HttpdConnection_t *conn,
                     int bits,
                     char *data,
                     size_t len,
                     void *device) {

	UNUSED(conn); 

#ifdef DEBUG
	fprintf(stdout, "Websocket [%d] got %lu bytes of ", (int)device, (unsigned long)len);
	switch (((unsigned char)bits) & 0x0F) {
	case MG_WEBSOCKET_OPCODE_CONTINUATION:
		fprintf(stdout, "continuation");
		break;
	case MG_WEBSOCKET_OPCODE_TEXT:
		fprintf(stdout, "text");
		break;
	case MG_WEBSOCKET_OPCODE_BINARY:
		fprintf(stdout, "binary");
		break;
	case MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE:
		fprintf(stdout, "close");
		break;
	case MG_WEBSOCKET_OPCODE_PING:
		fprintf(stdout, "ping");
		break;
	case MG_WEBSOCKET_OPCODE_PONG:
		fprintf(stdout, "pong");
		break;
	default:
		fprintf(stdout, "unknown(%1xh)", ((unsigned char)bits) & 0x0F);
		break;
	}
	fprintf(stdout, " data:\r\n");
	fwrite(data, len, 1, stdout);
	fprintf(stdout, "\r\n");
#endif

    if ((((unsigned char)bits) & 0x0F) == MG_WEBSOCKET_OPCODE_TEXT)

        switch ((net_device_t)device) {
        case DEV_SIO1:
        case DEV_VIO:
			if (len != 1) {
				LOGW(TAG, "Websocket recieved too many [%d] characters", (int)len);
				return 0;
			}
            msg.mtype = 1L;
            msg.mtext[0] = data[0];
            msg.mtext[1] = '\0';
            if (msgsnd(queue[(net_device_t)device], &msg, 2, 0)) {
                perror("msgsnd()");
            };
            break;
        default:
            break;
    }

	return 1;
}

void WebSocketCloseHandler(const HttpdConnection_t *conn, void *device) {
	struct mg_context *ctx = mg_get_context(conn);
	ws_client_t *client = mg_get_user_connection_data(conn);

	mg_lock_context(ctx);
	client->state = 0;
	client->conn = NULL;
	mg_unlock_context(ctx);
	
	LOGI(TAG, "WS CLIENT CLOSED %s", dev_name[(net_device_t) device]);

	if (queue[(net_device_t) device] && msgctl(queue[(net_device_t) device], IPC_RMID, NULL) == -1) {
		perror("msgctl()");
	}
	queue[(net_device_t) device] = 0;

	LOGD(TAG, "Message queue closed (%d) [%08X]", (net_device_t) device, queue[(net_device_t) device]);
}

static struct mg_context *ctx = NULL;

void stop_net_services (void) {

	if (ctx != NULL) {
		InformWebsockets(ctx);

		/* Stop the server */
		mg_stop(ctx);
		LOGI(TAG, "Server stopped.");
		LOGI(TAG, "Bye!");

		ctx = NULL;
	}
}

int start_net_services (void) {

	//TODO: add config for DOCUMENT_ROOT, PORT

	const char *options[] = {
	    "document_root",
	    DOCUMENT_ROOT,
	    "listening_ports",
	    PORT,
	    "request_timeout_ms",
	    "10000",
	    "error_log_file",
	    "error.log",
	    "websocket_timeout_ms",
	    "3600000",
	    "enable_auth_domain_check",
	    "no",
		"url_rewrite_patterns",
		"/imsai/disks/=./disks/, /imsai/conf/=./conf/, /imsai/printer.txt=./printer.txt",
	    0};

	struct mg_callbacks callbacks;

#ifdef DEBUG
	struct mg_server_ports ports[32];
	int port_cnt, n;
	int err = 0;
    const struct mg_option *opts;
#endif

	atexit(stop_net_services);

	memset(queue, 0, sizeof(queue));

    /* Start CivetWeb web server */
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.log_message = log_message;

	ctx = mg_start(&callbacks, 0, options);

	/* Check return value: */
	if (ctx == NULL) {
		LOGW(TAG, "Cannot start CivetWeb - mg_start failed.");
		return EXIT_FAILURE;
	}

	//TODO: sort out all the paths for the handlers
    mg_set_request_handler(ctx, "/system", 		SystemHandler, 		0);
    mg_set_request_handler(ctx, "/conf", 		ConfigHandler, 		"conf");
#ifdef HAS_DISKMANAGER
    mg_set_request_handler(ctx, "/library", 	LibraryHandler, 	0);
    mg_set_request_handler(ctx, "/disks", 		DiskHandler, 		0);
#endif

	mg_set_websocket_handler(ctx, "/sio1",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) DEV_SIO1);
	mg_set_websocket_handler(ctx, "/lpt",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) DEV_LPT);
	
	mg_set_websocket_handler(ctx, "/vio",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) DEV_VIO);
	
	mg_set_websocket_handler(ctx, "/cpa",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) DEV_CPA);

#ifdef DEBUG
	/* List all listening ports */
	memset(ports, 0, sizeof(ports));
	port_cnt = mg_get_server_ports(ctx, 32, ports);
	printf("\n%i listening ports:\n", port_cnt);
#endif

	return EXIT_SUCCESS;
}
#endif