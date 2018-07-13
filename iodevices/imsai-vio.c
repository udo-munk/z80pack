/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2017-2018 by Udo Munk
 *
 * Emulation of an IMSAI VIO S100 board
 *
 * History:
 * 10-JAN-17 80x24 display output tested and working
 * 11-JAN-17 implemented keyboard input for the X11 key events
 * 12-JAN-17 all resolutions in all video modes tested and working
 * 04-FEB-17 added function to terminate thread and close window
 * 21-FEB-17 added scanlines to monitor
 * 20-APR-18 avoid thread deadlock on Windows/Cygwin
 * 07-JUL-18 optimization
 * 12-JUL-18 use logging
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"
#include "log.h"
#include "imsai-vio-charset.h"

#define XOFF 10				/* use some offset inside the window */
#define YOFF 15				/* for the drawing area */

static const char *TAG = "VIO";

/* X11 stuff */
       int slf = 1;			/* scanlines factor, default no lines */
static int xsize, ysize;		/* window size */
static int xscale, yscale;
static int sx, sy;
static Display *display;
static Window window;
static int screen;
static GC gc;
static XWindowAttributes wa;
static Pixmap pixmap;
static Colormap colormap;
static XColor black, bg, fg;
static char black_color[] = "#000000";	/* black */
       char bg_color[] = "#303030";	/* default background color */
       char fg_color[] = "#FFFFFF";	/* default foreground color */
static XEvent event;
static KeySym key;
static char text[10];

/* VIO stuff */
static int state;			/* state on/off for refresh thread */
static int mode;		/* video mode written to command port memory */
static int modebuf;			/* and double buffer for it */
static int vmode, res, inv;		/* video mode, resolution & inverse */
int imsai_kbd_status, imsai_kbd_data;	/* keyboard status & data */

/* UNIX stuff */
static pthread_t thread;

/* create the X11 window for VIO display */
static void open_display(void)
{
	Window rootwindow;
	XSizeHints *size_hints = XAllocSizeHints();
	Atom wm_delete_window;

	xsize = 560 + (XOFF * 2);
	ysize = (240 * slf) + (YOFF * 2);

	display = XOpenDisplay(NULL);
	XLockDisplay(display);
	screen = DefaultScreen(display);
	rootwindow = RootWindow(display, screen);
	XGetWindowAttributes(display, rootwindow, &wa);
	window = XCreateSimpleWindow(display, rootwindow, 0, 0,
					xsize, ysize, 1, 0, 0);
	XStoreName(display, window, "IMSAI VIO");
	size_hints->flags = PSize | PMinSize | PMaxSize;
	size_hints->min_width = xsize;
	size_hints->min_height = ysize;
	size_hints->base_width = xsize;
	size_hints->base_height = ysize;
	size_hints->max_width = xsize;
	size_hints->max_height = ysize;
	XSetWMNormalHints(display, window, size_hints);
	XFree(size_hints);
	wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &wm_delete_window, 1);
	XSelectInput(display, window, KeyPressMask);
	colormap = DefaultColormap(display, 0);
	gc = XCreateGC(display, window, 0, NULL);
	pixmap = XCreatePixmap(display, rootwindow, xsize, ysize, wa.depth);

	XParseColor(display, colormap, black_color, &black);
	XAllocColor(display, colormap, &black);
	XParseColor(display, colormap, bg_color, &bg);
	XAllocColor(display, colormap, &bg);
	XParseColor(display, colormap, fg_color, &fg);
	XAllocColor(display, colormap, &fg);

	XMapWindow(display, window);
	XSync(display, True);
	XUnlockDisplay(display);
}

/* shutdown VIO thread and window */
void imsai_vio_off(void)
{
	state = 0;		/* tell refresh thread to stop */
	SLEEP_MS(50);		/* and wait a bit */

	/* works if X11 with posix threads implemented correct, but ... */
	if (thread != 0) {
		pthread_cancel(thread);
		pthread_join(thread, NULL);
	}

	if (display != NULL) {
		XLockDisplay(display);
		XFreePixmap(display, pixmap);
		XFreeGC(display, gc);
		XUnlockDisplay(display);
		XCloseDisplay(display);
	}
}

/* display characters 80-FF from bits 0-6, bit 7 = inverse video */
static void dc1(BYTE c)
{
	register int x, y;
	int cinv = (c & 128) ? 1 : 0;

	for (x = 0; x < 7; x++) {
		for (y = 0; y < 10; y++) {
			if (charset[(c << 1) & 0xff][y][x] == 1) {
				if ((cinv ^ inv) == 0)
				    XSetForeground(display, gc, fg.pixel);
				else
				    XSetForeground(display, gc, bg.pixel);
			} else {
				if ((cinv ^ inv) == 0)
				    XSetForeground(display, gc, bg.pixel);
				else
				    XSetForeground(display, gc, fg.pixel);
			}
			XDrawPoint(display, pixmap, gc, sx + (x * xscale),
				   sy + (y * yscale * slf));
			if (res & 1)
				XDrawPoint(display, pixmap, gc,
					   sx + (x * xscale) + 1,
					   sy + (y * yscale * slf));
			if (res & 2)
				XDrawPoint(display, pixmap, gc,
					   sx + (x * xscale),
					   sy + (y * yscale * slf) + (1 * slf));
			if ((res & 3) == 3)
				XDrawPoint(display, pixmap, gc,
					   sx + (x * xscale) + 1,
					   sy + (y * yscale * slf) + (1 * slf));
		}
	}
}

/* display characters 00-7F from bits 0-6, bit 7 = inverse video */
static void dc2(BYTE c)
{
	register int x, y;
	int cinv = (c & 128) ? 1 : 0;

	for (x = 0; x < 7; x++) {
		for (y = 0; y < 10; y++) {
			if (charset[c & 0x7f][y][x] == 1) {
				if ((cinv ^ inv) == 0)
				    XSetForeground(display, gc, fg.pixel);
				else
				    XSetForeground(display, gc, bg.pixel);
			} else {
				if ((cinv ^ inv) == 0)
				    XSetForeground(display, gc, bg.pixel);
				else
				    XSetForeground(display, gc, fg.pixel);
			}
			XDrawPoint(display, pixmap, gc, sx + (x * xscale),
				   sy + (y * yscale * slf));
			if (res & 1)
				XDrawPoint(display, pixmap, gc,
					   sx + (x * xscale) + 1,
					   sy + (y * yscale * slf));
			if (res & 2)
				XDrawPoint(display, pixmap, gc,
					   sx + (x * xscale),
					   sy + (y * yscale * slf) + (1 * slf));
			if ((res & 3) == 3)
				XDrawPoint(display, pixmap, gc,
					   sx + (x * xscale) + 1,
					   sy + (y * yscale * slf) + (1 * slf));
		}
	}
}

/* display characters 00-FF from bits 0-7, inverse video from command word */
static void dc3(BYTE c)
{
	register int x, y;

	for (x = 0; x < 7; x++) {
		for (y = 0; y < 10; y++) {
			if (charset[c][y][x] == 1) {
				if (inv == 0)
				    XSetForeground(display, gc, fg.pixel);
				else
				    XSetForeground(display, gc, bg.pixel);
			} else {
				if (inv == 0)
				    XSetForeground(display, gc, bg.pixel);
				else
				    XSetForeground(display, gc, fg.pixel);
			}
			XDrawPoint(display, pixmap, gc, sx + (x * xscale),
				   sy + (y * yscale * slf));
			if (res & 1)
				XDrawPoint(display, pixmap, gc,
					   sx + (x * xscale) + 1,
					   sy + (y * yscale * slf));
			if (res & 2)
				XDrawPoint(display, pixmap, gc,
					   sx + (x * xscale),
					   sy + (y * yscale * slf) + (1 * slf));
			if ((res & 3) == 3)
				XDrawPoint(display, pixmap, gc,
					   sx + (x * xscale) + 1,
					   sy + (y * yscale * slf) + (1 * slf));
		}
	}
}

/*
 * Check the X11 event queue, we are only interested in keyboard input.
 * Note that I'm using the event queue as typeahead buffer, saves to
 * implement one self.
 */
static inline void event_handler(void)
{
	/* if the last character wasn't processed already do nothing */
	/* keep event in queue until the CPU emulation got current one */
	if (imsai_kbd_status != 0)
		return;

	/* if there is a keyboard event get it and convert with keymap */
	if (XEventsQueued(display, QueuedAlready) > 0) {
		XNextEvent(display, &event);
		if ((event.type == KeyPress) &&
		    XLookupString(&event.xkey, text, 1, &key, 0) == 1) {
			imsai_kbd_data = text[0];
			imsai_kbd_status = 2;
		}
	}
}

/* refresh the display buffer dependend on video mode */
static void refresh(void)
{
	static int cols, rows;
	static BYTE c;
	register int x, y;

	sx = XOFF;
	sy = YOFF;

	mode = dma_read(0xf7ff);
	if (mode != modebuf) {
		modebuf = mode;

		vmode = (mode >> 2) & 3;
		res = mode & 3;
		inv = (mode & 16) ? 1 : 0;

		if (res & 1) {
			cols = 40;
			xscale = 2;
		} else {
			cols = 80;
			xscale = 1;
		}

		if (res & 2) {
			rows = 12;
			yscale = 2;
		} else {
			rows = 24;
			yscale = 1;
		}
	}

	switch (vmode) {
	case 0:	/* Video mode 0: video off, screen blanked */
		event_handler();
		XSetForeground(display, gc, black.pixel);
		XFillRectangle(display, pixmap, gc, 0, 0, xsize, ysize);
		break;

	case 1: /* Video mode 1: display character codes 80-FF */
		for (y = 0; y < rows; y++) {
			sx = XOFF;
			event_handler();
			for (x = 0; x < cols; x++) {
				c = dma_read(0xf000 + (y * cols) + x);
				dc1(c);
				sx += (res & 1) ? 14 : 7;
			}
			sy += (res & 2) ? 20 * slf : 10 * slf;
		}
		break;

	case 2:	/* Video mode 2: display character codes 00-7F */
		for (y = 0; y < rows; y++) {
			sx = XOFF;
			event_handler();
			for (x = 0; x < cols; x++) {
				c = dma_read(0xf000 + (y * cols) + x);
				dc2(c);
				sx += (res & 1) ? 14 : 7;
			}
			sy += (res & 2) ? 20 * slf : 10 * slf;
		}
		break;

	case 3:	/* Video mode 3: display character codes 00-FF */
		for (y = 0; y < rows; y++) {
			sx = XOFF;
			event_handler();
			for (x = 0; x < cols; x++) {
				c = dma_read(0xf000 + (y * cols) + x);
				dc3(c);
				sx += (res & 1) ? 14 : 7;
			}
			sy += (res & 2) ? 20 * slf : 10 * slf;
		}
		break;
	}
}

/* thread for updating the display */
static void *update_display(void *arg)
{
	extern int time_diff(struct timeval *, struct timeval *);

	struct timeval t1, t2;
	int tdiff;

	arg = arg;	/* to avoid compiler warning */
	gettimeofday(&t1, NULL);

	while (state) {

		/* lock display, don't cancel thread while locked */
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		XLockDisplay(display);

		/* update display window */
		refresh();
		XCopyArea(display, pixmap, window, gc, 0, 0,
			  xsize, ysize, 0, 0);
		XSync(display, False);

		/* unlock display, thread can be canceled again */
		XUnlockDisplay(display);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		/* sleep rest to 33ms so that we get 30 fps */
		gettimeofday(&t2, NULL);
		tdiff = time_diff(&t1, &t2);
		if ((tdiff > 0) && (tdiff < 33000))
			SLEEP_MS(33 - (tdiff / 1000));

		gettimeofday(&t1, NULL);
	}

	pthread_exit(NULL);
}

/* create the X11 window and start display refresh thread */
void imsai_vio_init(void)
{
	open_display();

	state = 1;
	modebuf = -1;
	dma_write(0xf7ff, 0x00);

	if (pthread_create(&thread, NULL, update_display, (void *) NULL)) {
		LOGE(TAG, "can't create thread");
		exit(1);
	}
}
