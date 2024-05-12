/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2017-2019 by Udo Munk
 *
 * Emulation of a Processor Technology VDM-1 S100 board
 *
 * History:
 * 28-FEB-2017 first version, all software tested with working
 * 21-JUN-2017 don't use dma_read(), switches Tarbell ROM off
 * 20-APR-2018 avoid thread deadlock on Windows/Cygwin
 * 15-JUL-2018 use logging
 * 04-NOV-2019 eliminate usage of mem_base()
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "sim.h"
#include "simglb.h"
#include "frontpanel.h"
#include "memory.h"
#include "log.h"
#include "proctec-vdm-charset.h"

#define XOFF 10				/* use some offset inside the window */
#define YOFF 15				/* for the drawing area */

static const char *TAG = "VDM";

/* X11 stuff */
       int slf = 1;			/* scanlines factor, default no lines */
static int xsize, ysize;		/* window size */
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

/* VDM stuff */
static int state;			/* state on/off for refresh thread */
static int mode;			/* video mode from I/O port */
int proctec_kbd_status = 1;		/* keyboard status */
int proctec_kbd_data = -1;		/* keyboard data */
static int first;			/* first displayed screen position */
static int beg;				/* beginning display line address */

/* UNIX stuff */
static pthread_t thread;

/* create the X11 window for VDM display */
static void open_display(void)
{
	Window rootwindow;
	XSizeHints *size_hints = XAllocSizeHints();
	Atom wm_delete_window;

	xsize = 576 + (XOFF * 2);
	ysize = (208 * slf) + (YOFF * 2);

	display = XOpenDisplay(NULL);
	XLockDisplay(display);
	screen = DefaultScreen(display);
	rootwindow = RootWindow(display, screen);
	XGetWindowAttributes(display, rootwindow, &wa);
	window = XCreateSimpleWindow(display, rootwindow, 0, 0,
				     xsize, ysize, 1, 0, 0);
	XStoreName(display, window, "Processor Technology VDM-1");
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
	XSetForeground(display, gc, black.pixel);
	XFillRectangle(display, pixmap, gc, 0, 0, xsize, ysize);
	XSync(display, True);
	XUnlockDisplay(display);
}

/* shutdown VDM thread and window */
void proctec_vdm_off(void)
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

/*
 * Check the X11 event queue, we are only interested in keyboard input.
 * Note that I'm using the event queue as typeahead buffer, saves to
 * implement one self.
 */
static inline void event_handler(void)
{
	/* if the last character wasn't processed already do nothing */
	/* keep event in queue until the CPU emulation got current one */
	if (proctec_kbd_status == 0)
		return;

	/* if there is a keyboard event get it and convert with keymap */
	if (XEventsQueued(display, QueuedAlready) > 0) {
		XNextEvent(display, &event);
		if ((event.type == KeyPress) &&
		    XLookupString(&event.xkey, text, 1, &key, 0) == 1) {
			proctec_kbd_data = text[0];
			proctec_kbd_status = 0;
		}
	}
}

/* display characters, bit 7 = inverse video */
static void dc(BYTE c)
{
	register int x, y;
	int inv = (c & 128) ? 1 : 0;

	for (x = 0; x < 9; x++) {
		for (y = 0; y < 13; y++) {
			if (charset[c & 0x7f][y][x] == 1) {
				if (!inv)
					XSetForeground(display, gc, fg.pixel);
				else
					XSetForeground(display, gc, bg.pixel);
			} else {
				if (!inv)
					XSetForeground(display, gc, bg.pixel);
				else
					XSetForeground(display, gc, fg.pixel);
			}
			XDrawPoint(display, pixmap, gc, sx + x, sy + (y * slf));
		}
	}
}

/* refresh the display buffer */
static void refresh(void)
{
	register int x, y;
	static int addr;
	static BYTE c;

	sy = YOFF;
	addr = 0xcc00 + beg * 64;

	for (y = 0; y < 16; y++) {
		sx = XOFF;
		event_handler();
		for (x = 0; x < 64; x++) {
			if (y >= first) {
				c = getmem(addr + x);
				dc(c);
			} else
				dc(' ');
			sx += 9;
		}
		sy += 13 * slf;
		addr += 64;
		if (addr >= 0xd000)
			addr = 0xcc00;
	}
}

/* thread for updating the display */
static void *update_display(void *arg)
{
	extern unsigned long long get_clock_us(void);

	unsigned long long t1, t2;
	int tdiff;

	UNUSED(arg);

	t1 = get_clock_us();

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
		t2 = get_clock_us();
		tdiff = t2 - t1;
		if ((tdiff > 0) && (tdiff < 33000))
			SLEEP_MS(33 - (tdiff / 1000));

		t1 = get_clock_us();
	}

	pthread_exit(NULL);
}

/* create the X11 window and start display refresh thread */
static void vdm_init(void)
{
	open_display();

	state = 1;

	if (pthread_create(&thread, NULL, update_display, (void *) NULL)) {
		LOGE(TAG, "can't create thread");
		exit(EXIT_FAILURE);
	}
}

/* I/O port for the VDM */
void proctec_vdm_out(BYTE data)
{
	mode = data;
	first = (data & 0xf0) >> 4;
	beg = data & 0x0f;

	if (display == 0)
		vdm_init();

}
