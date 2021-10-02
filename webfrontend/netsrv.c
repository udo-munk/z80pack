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
#include <sys/stat.h>
#include <sys/utsname.h>
#include "sim.h"
#include "simglb.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"
#include "log.h"
#include "netsrv.h"
#include "civetweb.h"

#ifdef HAS_HAL
#include "imsai-hal.h"
#endif 

#ifdef HAS_NETSERVER

#define PORT "8080"

#define MAX_WS_CLIENTS (_DEV_MAX)
static const char *TAG = "netsrv";

static msgbuf_t msg;

struct {
	int queue;
    ws_client_t ws_client;
    void (*cbfunc)(BYTE *);
} dev[MAX_WS_CLIENTS];

net_device_t net_device_a[_DEV_MAX] = { 
	DEV_TTY, DEV_LPT, DEV_VIO, DEV_CPA, 
	DEV_DZLR, DEV_88ACC, DEV_D7AIO, DEV_PTR 
};

char *dev_name[] = {
	"TTY",
	"LPT",
	"VIO",
	"CPA",
	"DZLR",
	"ACC",
	"D7AIO",
	"PTR"
};

int last_error = 0; //TODO: replace
/*
extern int reset;
extern int power;
extern void quit_callback(void);
*/

extern void lpt_reset(void);

#ifdef HAS_DISKMANAGER
extern int LibraryHandler(struct mg_connection *, void *);
extern int DiskHandler(struct mg_connection *, void *);
#endif

/**
 * Check if a queue is provisioned
 */
int net_device_alive(net_device_t device) {
	return dev[device].queue;
}

void net_device_service(net_device_t device, void (*cbfunc)(BYTE *data)) {
    dev[device].cbfunc = cbfunc;
}

/**
 * Assumes the data is:
 * 		TEXT	if only a single byte
 * 		BINARY	if there are multiple bytes
 * 		TTY & LPT are always BINARY now
 */
void net_device_send(net_device_t device, char* msg, int len) {

	int op_code;

	switch (device) {
		case DEV_TTY:
		case DEV_PTR:
		case DEV_LPT:
			op_code = MG_WEBSOCKET_OPCODE_BINARY;
			break;
		default:
			op_code = (len==1)?MG_WEBSOCKET_OPCODE_TEXT:MG_WEBSOCKET_OPCODE_BINARY;
			break;
	}

 	if(dev[device].queue) {
		mg_websocket_write(dev[device].ws_client.conn,
			op_code,
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

	if (dev[device].queue) {
		res = msgrcv(dev[device].queue, &msg, 2, 1L, IPC_NOWAIT);
		LOGD(TAG, "GET: device[%d] res[%ld] msg[%ld, %s]\r\n", device, res, msg.mtype, msg.mtext);
		if (res == 2) {
			return msg.mtext[0];
		}
	}

	return -1;
}

int net_device_get_data(net_device_t device, char *dst, int len) {
	ssize_t res;
	msgbuf_t msg;

	if (dev[device].queue) {
		res = msgrcv(dev[device].queue, &msg, len, 1L, MSG_NOERROR);
		memcpy((void *)dst, (void *)msg.mtext, res);
		return res;
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

	if (dev[device].queue) {
		res = msgrcv(dev[device].queue, &msg, 1, 1L, IPC_NOWAIT);
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
		req.method = HTTP_UNKNOWN;
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
		if (dev[i].ws_client.state == 2) {
			mg_websocket_write(dev[i].ws_client.conn,
			                   MG_WEBSOCKET_OPCODE_TEXT,
			                   text,
			                   strlen(text));
		}
	}
	mg_unlock_context(ctx);
}

struct utsname uts;

int SystemHandler(HttpdConnection_t *conn, void *unused) {
    request_t *req = get_request(conn);
	UNUSED(unused);

	char *copyright = USR_CPR; /* a dirty fix to avoid the leading '\n' */

    switch(req->method) {
    case HTTP_GET:
		LOGD(TAG, "Sending SYS: details.");

		if (req->args[0] && *req->args[0] == 'm') {
            
            httpdStartResponse(conn, 200); 
            httpdHeader(conn, "Content-Type", "application/json");
            httpdEndHeaders(conn);

            httpdPrintf(conn, "{ \"machine\": \"" MACHINE "\" }");
            
            return 1;
        }

        httpdStartResponse(conn, 200); 
        httpdHeader(conn, "Content-Type", "application/json");
        httpdEndHeaders(conn);
		
		uname(&uts);

        httpdPrintf(conn, "{");

            httpdPrintf(conn, "\"machine\": \"" MACHINE "\", ");

            httpdPrintf(conn, "\"platform\": \"%s\", ", uts.sysname);
            
            httpdPrintf(conn, "\"network\": { ");

                httpdPrintf(conn, "\"hostname\": \"%s\" ", uts.nodename);

            httpdPrintf(conn, "}, ");

            httpdPrintf(conn, "\"paths\": { ");
                httpdPrintf(conn, "\"%s\": \"%s\", ", "CONFDIR", confdir);
                httpdPrintf(conn, "\"%s\": \"%s\", ", "DISKSDIR", diskdir);
                httpdPrintf(conn, "\"%s\": \"%s\" ", "BOOTROM", rompath);
            httpdPrintf(conn, "}, ");

            httpdPrintf(conn, "\"system\": { ");
                httpdPrintf(conn, "\"%s\": \"%s\",", "VER", uts.version);
                httpdPrintf(conn, "\"%s\": \"%s\",", "MACHINE", uts.machine);
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
                httpdPrintf(conn, "\"%s\": \"%s\", ", "USR_CPR", &copyright[1]); 
                httpdPrintf(conn, "\"%s\": \"%s\", ", "cpu", cpu==Z80?"Z80":"I8080");
                if(x_flag) {
                    httpdPrintf(conn, "\"%s\": \"%s\", ", "bootrom", xfn);
                }
#ifdef FRONTPANEL
                    httpdPrintf(conn, "\"%s\": %d, ", "cpa", 1);
#endif
                httpdPrintf(conn, "\"%s\": %d ", "clock", f_flag);
            httpdPrintf(conn, "} ");

#ifdef HAS_HAL
            httpdPrintf(conn, ", \"sio_ports\": [ ");
            for(int i = 0; i < MAX_SIO_PORT; i++) {
                httpdPrintf(conn, "{ ");
                    httpdPrintf(conn, "\"%s\": \"%s\", ", "name", sio_port_name[i]);
                    int j = 0;
                    httpdPrintf(conn, "\"%s\": [ ", "devices");
                    while (sio[i][j].name && j < MAX_HAL_DEV) {
                        if (*sio[i][j].name) 
							httpdPrintf(conn, "%s\"%s%s\"", 
								(j==0)?"":", ", 
								sio[i][j].name, 
								sio[i][j].fallthrough?"+":""
							);
                        j++;
                    }
                httpdPrintf(conn, "] }%s ", (i < (MAX_SIO_PORT-1))?",":"");
            }
            httpdPrintf(conn, "]");
#endif 

#ifdef HAS_CONFIG
            httpdPrintf(conn, ", \"memmap\": [ ");

				for (int i=0; i < MAXMEMMAP; i++) {
					if (memconf[M_flag][i].size) {


						if (i > 0) httpdPrintf(conn, ", ");
						httpdPrintf(conn, "{ \"type\": \"%s\"", (memconf[M_flag][i].type==MEM_RW)?"RAM":"ROM");
						httpdPrintf(conn, ", \"from\": %d", memconf[M_flag][i].spage << 8);
						httpdPrintf(conn, ", \"to\": %d", (memconf[M_flag][i].spage << 8) + (memconf[M_flag][i].size << 8) - 1);
						if (memconf[M_flag][i].type==MEM_RO && memconf[M_flag][i].rom_file) 
							httpdPrintf(conn, ", \"file\": \"%s\"", memconf[M_flag][i].rom_file);
						httpdPrintf(conn, "}");
					}
				}

            httpdPrintf(conn, "]");

            httpdPrintf(conn, ", \"memextra\": [ ");

			if (boot_switch[M_flag]) {
            	httpdPrintf(conn, "\"Power-on jump address %04XH\", ", boot_switch[M_flag]);
			}
			if (R_flag) {
            	httpdPrintf(conn, "\"%s\", ", BANKED_ROM_MSG);
			}

			extern int num_banks;
			if (num_banks) {
            	httpdPrintf(conn, "\"MMU has %d additional RAM banks of %d KB\",", num_banks, SEGSIZ >> 10);
			}

            httpdPrintf(conn, " \"\" ]");
#endif
			int i=0, o=0;
            char *t1, *t2;
			extern char **environ;
			char buf[2048];

            httpdPrintf(conn, ", \"env\": { ");
                while(environ[i] != NULL) {
                    strcpy(buf, environ[i]);
                    t1 = strtok(buf, "=");
                    t2 = strtok(NULL, "\0");
#define BULLET  "\xE2\x80\xA2"
                    if(!strcmp(t1, "PASSWORD") && (getenv("WIFI.password.hide") != NULL)) 
                        t2 = BULLET BULLET BULLET BULLET BULLET BULLET BULLET BULLET;
					/* Filter out only TERM and non-shell environment valiables of the form '*.*' ie. contain '.' */
					if(!strcmp(t1, "TERM") || index(t1, '.'))
                    	httpdPrintf(conn, "%s \"%s\": \"%s\" ", (o++)==0?"":",", t1, (t2==NULL)?"":t2);
                    i++;
                }
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
    struct stat sb;
    char fullpath[MAX_LFN];
    bool showSize = false;
			
    if (req->args[0] && *req->args[0] == 'S') {
    	showSize = true;
    }

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
		snprintf(&fullpath[0], MAX_LFN, "%s/%s", (char *) path, pDirent->d_name);
		/*
		 * not working with some filesystems like Linux xfs, need to use stat()
                 * if (pDirent->d_type==DT_REG) {
		 */
		if (stat(fullpath, &sb) != -1 && S_ISREG(sb.st_mode)) {
			if (showSize) {
				httpdPrintf(conn, "%c{ \"%s\":\"%s\",", (i++ > 0)?',':' ', "filename",pDirent->d_name);
				httpdPrintf(conn, "\"%s\":%lld}", "size", sb.st_size);
			} else {
				httpdPrintf(conn, "%c\"%s\"", (i++ > 0)?',':' ', pDirent->d_name);
			}
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
	net_device_t d = *(net_device_t *) device;

	mg_lock_context(ctx);
		if (dev[d].ws_client.conn == NULL) {
			dev[d].ws_client.conn = (struct mg_connection *)conn;
			dev[d].ws_client.state = 1;
			mg_set_user_connection_data(dev[d].ws_client.conn, (void *)(&(dev[d].ws_client)));

			switch (d) {
			case DEV_TTY:
			case DEV_PTR:
			case DEV_VIO:
			case DEV_LPT:
			case DEV_DZLR:
			case DEV_88ACC:
			case DEV_D7AIO:
				res = msgget(IPC_PRIVATE, 0644 | IPC_CREAT); //TODO: check flags
				if (res > 0) {
					dev[d].queue = res;
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
	net_device_t d = *(net_device_t *) device;

	if (d == DEV_TTY) 
		mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, text, strlen(text));
	
	if (d == DEV_VIO) {
		BYTE mode = dma_read(0xf7ff);
		dma_write(0xf7ff, 0x00);
		SLEEP_MS(100);
		dma_write(0xf7ff, mode);
	}

	LOGI(TAG, "WS CLIENT CONNECTED to %s", dev_name[d]);

	client->state = 2;
}

int WebsocketDataHandler(HttpdConnection_t *conn,
                     int bits,
                     char *data,
                     size_t len,
                     void *device) {

	net_device_t d = *(net_device_t *) device;
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

    if ((((unsigned char)bits) & 0x0F) == MG_WEBSOCKET_OPCODE_BINARY) {

        switch (d) {
		case DEV_D7AIO:
			if (dev[DEV_D7AIO].cbfunc != NULL && (len == 8)) {
                (*(dev[DEV_D7AIO].cbfunc))((BYTE *)data);
            }
			break;
		case DEV_PTR:
			if (len != 1) {
				LOGW(TAG, "Websocket recieved too many [%d] characters", (int)len);
				return 0;
			}
            msg.mtype = 1L;
            msg.mtext[0] = data[0];
            msg.mtext[1] = '\0';
            if (msgsnd(dev[d].queue, &msg, 2, IPC_NOWAIT)) {
                if (errno == EAGAIN) {
					LOGW(TAG, "%s Overflow", dev_name[d]);
				} else {
					perror("msgsnd()");
				}
				return 0;
            };
            break;	
		case DEV_88ACC:
			// LOGI(TAG, "rec: %d, %d", (int)len, (BYTE)*data);
            msg.mtype = 1L;
            memcpy(msg.mtext, data, len);
			if (msgsnd(dev[d].queue, &msg, len, IPC_NOWAIT)) {
                if (errno == EAGAIN) {
					LOGW(TAG, "%s Overflow", dev_name[d]);
				} else {
					perror("msgsnd()");
				}
				return 0;
            };
			break;
		default:
			break;
		};
	}
    if ((((unsigned char)bits) & 0x0F) == MG_WEBSOCKET_OPCODE_TEXT) {

        switch (d) {
		case DEV_LPT:
			if (len == 1 && *data == 'R') lpt_reset();
			break;
        case DEV_TTY:
        case DEV_VIO:
			if (len != 1) {
				LOGW(TAG, "Websocket recieved too many [%d] characters", (int)len);
				return 0;
			}
            msg.mtype = 1L;
            msg.mtext[0] = data[0];
            msg.mtext[1] = '\0';
            if (msgsnd(dev[d].queue, &msg, 2, IPC_NOWAIT)) {
                if (errno == EAGAIN) {
					LOGW(TAG, "%s Overflow", dev_name[d]);
				} else {
					perror("msgsnd()");
				}
				return 0;
            };
            break;
        default:
            break;
		};
    }

	return 1;
}

void WebSocketCloseHandler(const HttpdConnection_t *conn, void *device) {
	struct mg_context *ctx = mg_get_context(conn);
	ws_client_t *client = mg_get_user_connection_data(conn);
	net_device_t d = *(net_device_t *) device;

	mg_lock_context(ctx);
	client->state = 0;
	client->conn = NULL;
	mg_unlock_context(ctx);
	
	LOGI(TAG, "WS CLIENT CLOSED %s", dev_name[d]);

	if (dev[d].queue && msgctl(dev[d].queue, IPC_RMID, NULL) == -1) {
		perror("msgctl()");
	}
	dev[d].queue = 0;

	LOGD(TAG, "Message queue closed (%d) [%08X]", d, dev[d].queue);
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
		"/" MACHINE "/disks/=./disks/, /" MACHINE "/conf/=./conf/, /" MACHINE "/printer.txt=./printer.txt",
	    0};

	struct mg_callbacks callbacks;

#ifdef DEBUG
	struct mg_server_ports ports[32];
	int port_cnt, n;
	int err = 0;
    const struct mg_option *opts;
#endif

	atexit(stop_net_services);

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

	mg_set_websocket_handler(ctx, "/tty",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) &net_device_a[DEV_TTY]);

	mg_set_websocket_handler(ctx, "/ptr",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) &net_device_a[DEV_PTR]);
							 
	mg_set_websocket_handler(ctx, "/lpt",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) &net_device_a[DEV_LPT]);
	
	mg_set_websocket_handler(ctx, "/vio",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) &net_device_a[DEV_VIO]);
	
	mg_set_websocket_handler(ctx, "/dazzler",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) &net_device_a[DEV_DZLR]);
	
	mg_set_websocket_handler(ctx, "/cpa",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) &net_device_a[DEV_CPA]);

	mg_set_websocket_handler(ctx, "/acc",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) &net_device_a[DEV_88ACC]);

	mg_set_websocket_handler(ctx, "/d7aio",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         (void *) &net_device_a[DEV_D7AIO]);

#ifdef DEBUG
	/* List all listening ports */
	memset(ports, 0, sizeof(ports));
	port_cnt = mg_get_server_ports(ctx, 32, ports);
	printf("\n%i listening ports:\n", port_cnt);
#endif

	return EXIT_SUCCESS;
}
#endif
