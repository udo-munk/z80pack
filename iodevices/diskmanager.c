/**
 * diskmanager.c
 *
 * Copyright (C) 2018 by David McNaughton
 * 
 * History:
 * 12-JUL-18    1.0     Initial Release
 */

/**
 * This diskmanager module provides an alternate interface for the array 
 * 
 *      char *disks[];
 * 
 * that is typically implemented in a disk controller simulators e.g.
 *      - imsai-fif.c
 *      - tarbell_fdc.c
 * 
 * It persists the array in a file, typically "disk.map" [DISKMAP] in the 
 * provided path.
 * 
 * The "disk.map" file is a simple text file with a line for each disk
 * starting at 'A' upto [LAST_DISK], typically 'D'.
 *      - If a line is empty of starts with '#' the disk is "ejected"
 *      - If a disk image is "inserted" the line conatains only the file name
 * 
 * The diskmanager provides functions to:
 *      - populate the array from the file
 *      - write the array to the file
 *      - insert a disk
 *          - stat() disk image files to validate them before inserting
 *          - reject inserting the same disk image in 2 disk drives
 *      - eject a disk
 *      - and some other support functions.
 * 
 * TODO:
 *      - fully support paths to allow a disk libabry hierarchy 
 *      - support more complex disk arrays that conatin structures e.g.
 *          * cpmsim::iosim.c
 *          * cromemco-fdc.c
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#define LOCAL_LOG_LEVEL LOG_DEBUG
#include "log.h"
#include "sim.h"
#ifdef HAS_NETSERVER
#include "civetweb.h"
#include "netsrv.h"
#endif


#ifdef HAS_DISKMANAGER
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef DISKMAP
#define DISKMAP     "disk.map"
#endif

#define LAST_DISK   'D'
#define _MAX_DISK   (LAST_DISK - '@')

static const char *TAG = "diskmanager";

char *disks[_MAX_DISK];
extern char *disks[];

static char path[MAX_LFN+1];		/* path/filename for disk image */
static char *file_start;
#define APPENDTOPATH(file) strncpy(file_start, file, MAX_LFN - strlen(path));

enum disk_err {
    SUCCESS,
    INVALID_DISK_NUM,
    DRIVE_EMPTY,
    DRIVE_NOT_EMPTY,
    IMAGE_ALREADY_INSERTED,
    IMAGE_NOT_VALID,
    FAILURE
};

typedef enum disk_err disk_err_t;

int findDiskImage(const char *image) {
    int i;

    for (i = 0; i < _MAX_DISK; i++) {
        if (disks[i] != NULL) {
            if (strcmp(image, disks[i]) == 0) {
                return 1;                  
            }
        }   
    }
    return 0;
}

int getDiskNumByID(const char *id) {
    int disk = -1;
 
    /* id is in the form X:DSK: making it 6 characters long */
    LOGD(TAG, "GET ID: %s, %ld", id, strlen(id));
    if (id != NULL && strlen(id) == 6 && strcmp(id+1, ":DSK:") == 0) {
        disk = *id - 'A';
        if (disk < 0 || disk > (_MAX_DISK - 1)) {
            LOGW(TAG, "BAD DISK ID: %c", *id);   
            disk = -1;
        }
    } else {
        LOGW(TAG, "DISK ID required, got: %s", id);
        disk = -1;   
    }
    return disk;
}

disk_err_t insertDisk(int disk, const char *image) {

    char *name;
    struct stat image_status;
    int i;

    if (disk >= 0 && disk < _MAX_DISK) {

        if (disks[disk] != NULL) {
            return DRIVE_NOT_EMPTY;
        }

        if (image != NULL && strlen(image) < MAX_LFN) {

            for (i = 0; i < _MAX_DISK; i++) {
                if (disks[i] != NULL && strcmp(image, disks[i]) == 0) {
                    return IMAGE_ALREADY_INSERTED;
                }
            }

            APPENDTOPATH(image);

            if (stat(path, &image_status) == 0) {
                if (S_ISREG(image_status.st_mode)) {
                    name = strndup(image, MAX_LFN);
                    if (name == NULL) {
                        LOGW(TAG, "Failed to insert disk, not enough memory.");
                        return FAILURE;
                    } else {
                        /* Everything is OK, we can insert the disk */
                        disks[disk] = name;
                        return SUCCESS;
                    }
                } else {
                    return IMAGE_NOT_VALID;
                }
            } else {
                LOGW(TAG, "Failed to stat disk image: %s, error: %d", image, errno);
                return IMAGE_NOT_VALID;
            }
        }

        return IMAGE_NOT_VALID;
    }

    return INVALID_DISK_NUM;
}

disk_err_t ejectDisk(int disk) {

    char *name;

    if (disk >= 0 || disk < _MAX_DISK) {

        if (disks[disk] == NULL) {
            return DRIVE_EMPTY;
        }

        name = disks[disk];
        disks[disk] = NULL;
        free(name);
        
        return SUCCESS;
    }

    return INVALID_DISK_NUM;
}

void writeDiskmap(void) {
    FILE *map;
    int i;

    if (*path == '\0') {
         LOGW(TAG, "Path to disk map not set. Call readDiskmap(path) first.");
        return;       
    }

    APPENDTOPATH(DISKMAP);

    map = fopen(path, "w");
    if (map == NULL) {
        LOGW(TAG, "Can't create disk map: %s", path);
        return;
    }

    for (i = 0; i < _MAX_DISK; i++) {
        fprintf(map, "%s\n", disks[i]==NULL?"#":disks[i]);
    }
    fclose(map);  
}

void readDiskmap(char *path_name) {
    FILE *map;
    char *line = NULL;
    char *name;
    size_t len;
    ssize_t res;
    int i;
    disk_err_t insert;

    for (i = 0; i < _MAX_DISK; i++) {
        disks[i] = NULL;
    }

    strncpy(path, path_name, MAX_LFN);
    strncat(path, "/", MAX_LFN - strlen(path));
    file_start = path + strlen(path);

    APPENDTOPATH(DISKMAP);
    LOGD(TAG, "LIB: path: %s, diskmap: %s", path, file_start);

    i = 0;
again:
    map = fopen(path, "r");
    if (map == NULL) {
        if (i++ == 0) {
            LOGW(TAG, "No disk map: %s, attempting to create file.", path);
            writeDiskmap();
            goto again;
        }
        return;
    }

    LOG(TAG, "DISK MAP: [%s]\r\n", path);

    for (i = 0; i < _MAX_DISK; i++) {
        line = NULL;
        res = getline(&line, &len, map);

        if (res != -1) {
            if (line[res-1] == '\n') 
                line[res-1] = '\0';

            /* empty lines or lines that begin with # are empty disks */
            name = ((line[0]=='\0') || (line[0]=='#'))?NULL:line;
            if (name != NULL) {
                insert = insertDisk(i, name);

                switch (insert) {
                    case SUCCESS :
                        LOG(TAG, "%c:DSK:='%s'\r\n", i+'A', disks[i]);
                        break;
                    case IMAGE_ALREADY_INSERTED :
                        LOGW(TAG, "%c:DSK: Image file '%s' already in use", i+'A', name);
                        break;
                    case IMAGE_NOT_VALID :
                        LOGW(TAG, "%c:DSK: Image file '%s' is invalid", i+'A', name);
                        break;
                    default:
                        LOGW(TAG, "%c:DSK: Failed to insert disk, error: %d", i+'A', insert)
                        break;
                }
            } 
        } 
        free(line);
    }
    fclose(map);
}

#ifdef HAS_NETSERVER
/**
 * Web Server handlers for LIB: and X:DSK:
 * 
 */
extern int DirectoryHandler(struct mg_connection *, void *);
extern int UploadHandler(struct mg_connection *, void *);

int LibraryHandler(HttpdConnection_t *conn, void *unused) {
    request_t *req = get_request(conn);
	int i = 0;
    UNUSED(unused);

    *file_start = '\0';

    if (*path == '\0') {
         LOGW(TAG, "Path to disk map not set. Call readDiskmap(path) first.");
        return 0;      
    }

    switch(req->method) {
    case HTTP_GET:
        DirectoryHandler(conn, path);
        break;
    case HTTP_PUT:
        UploadHandler(conn, path);
        LOGI(TAG, "PUT image: image uploaded.");
        break;
    case HTTP_DELETE:
        if (req->len > 0) {
            i = mg_read(conn, file_start, MAX_LFN - strlen(path));
            *(file_start + i) = '\0';

            LOGD(TAG, "DELETE image: %s, length: %lld", file_start, req->len);   
 
            if (findDiskImage(file_start)) {
                LOGW(TAG, "DELETE image: %s, currently inserted in disks", file_start);   
                httpdStartResponse(conn, 404);  /* http error code 'Not Found' */
                httpdEndHeaders(conn);   
                return 1;                  
            }

            if (unlink(path) < 0) {
                LOGW(TAG, "DELETE image: %s, unlink failed [%d]", path, errno);   
                httpdStartResponse(conn, 410);  /* http error code 'Gone' */
                httpdEndHeaders(conn);   
            } else {
                LOGI(TAG, "DELETE image: %s, deleted.", path);
                httpdStartResponse(conn, 200); 
                httpdEndHeaders(conn);
                httpdPrintf(conn, "Deleted");
            };
        }
        break;
    default:
        httpdStartResponse(conn, 405);  /* http error code 'Method Not Allowed' */
        httpdEndHeaders(conn);
        break;
    }
	return 1;
}

static void sendDisks(struct mg_connection *conn) {

    int i;
    
    httpdStartResponse(conn, 200); 
    httpdHeader(conn, "Content-Type", "application/json");
    httpdEndHeaders(conn);

    httpdPrintf(conn, "{");

    for (i = 0; i < _MAX_DISK; i++) {
        httpdPrintf(conn, "\"%c\": \"%s\"", i+'A', disks[i]==NULL?"":disks[i]);
        if (i < (_MAX_DISK - 1)) httpdPrintf(conn, ",");
    }

    httpdPrintf(conn, "}");          
}

int DiskHandler(HttpdConnection_t *conn, void *unused) {
    request_t *req = get_request(conn);
    int disk, i;
    char image[MAX_LFN];
    disk_err_t result;
    UNUSED(unused);
 
    switch(req->method) {
    case HTTP_GET:
        LOGD(TAG, "GET /disks");
        sendDisks(conn);
        break;
    case HTTP_PUT:
        LOGD(TAG, "PUT /disks: %s", req->args[0]);

        disk = getDiskNumByID(req->args[0]);
        image[0] = '\0';

        if (req->len > 0) {
            i = mg_read(conn, image, MAX_LFN);
            image[i] = '\0';
        };

        LOGD(TAG, "PUT image length: %d [%s]", (int) req->len, image);   

        result = insertDisk(disk, image);

        switch (result) {
            case SUCCESS :
                writeDiskmap();
                sendDisks(conn);
                break;
            case DRIVE_NOT_EMPTY :
                LOGW(TAG, "PUT /disks NOT EMPTY");   
                httpdStartResponse(conn, 404);  /* http error code 'Not Found' */
                httpdEndHeaders(conn); 
                break;
            case IMAGE_ALREADY_INSERTED :
                LOGW(TAG, "PUT image: %s, already inserted in disks", image);   
                httpdStartResponse(conn, 404);  /* http error code 'Not Found' */
                httpdEndHeaders(conn);   
                break;
            default:
                LOGW(TAG, "PUT image: %s, failed to insert disk: %d, error: %d", image, disk, result)
                httpdStartResponse(conn, 404);  /* http error code 'Not Found' */
                httpdEndHeaders(conn);   
                break;
        }
        break;
    case HTTP_DELETE:
        LOGD(TAG, "DELETE /disks: %s", req->args[0]);

        disk = getDiskNumByID(req->args[0]);
        result = ejectDisk(disk);

        switch (result) {
            case SUCCESS :
                sendDisks(conn);
                writeDiskmap();
                break;
            case DRIVE_EMPTY :
                LOGW(TAG, "DELETE /disks ALREADY EMPTY");
                httpdStartResponse(conn, 404);  /* http error code 'Not Found' */
                httpdEndHeaders(conn);    
                break;
            default:
                LOGW(TAG, "DELETE /disks failed to eject disk: %d, error: %d", disk, result)
                httpdStartResponse(conn, 404);  /* http error code 'Not Found' */
                httpdEndHeaders(conn);   
                break;
        }
        break;
    default:
        httpdStartResponse(conn, 405);  /* http error code 'Method Not Allowed' */
        httpdEndHeaders(conn);
	}

    return 1;   
}
#endif
#endif
