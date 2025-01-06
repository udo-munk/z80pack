/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2017-2019 by Udo Munk
 * Copyright (C) 2018 David McNaughton
 * Copyright (C) 2025 by Thomas Eberhardt
 *
 * Emulation of an IMSAI VIO S100 board
 *
 * History:
 * 10-JAN-2017 80x24 display output tested and working
 * 11-JAN-2017 implemented keyboard input for the X11 key events
 * 12-JAN-2017 all resolutions in all video modes tested and working
 * 04-FEB-2017 added function to terminate thread and close window
 * 21-FEB-2017 added scanlines to monitor
 * 20-APR-2018 avoid thread deadlock on Windows/Cygwin
 * 07-JUL-2018 optimization
 * 12-JUL-2018 use logging
 * 14-JUL-2018 integrate webfrontend
 * 05-NOV-2019 use correct memory access function
 * 04-JAN-2025 add SDL2 support
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef WANT_SDL
#include <stdbool.h>
#include <SDL.h>
#else
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"
#include "simport.h"
#include "simsdl.h"

#ifdef HAS_NETSERVER
#include "netsrv.h"
#endif

#if !defined(WANT_SDL) || defined(HAS_NETSERVER)
#include <pthread.h>
#include "log.h"
static const char *TAG = "VIO";
#endif

#include "imsai-vio-charset.h"
#include "imsai-vio.h"

#define XOFF 10				/* use some offset inside the window */
#define YOFF 15				/* for the drawing area */

/* SDL2/X11 stuff */
       int slf = 1;			/* scanlines factor, default no lines */
static int xsize, ysize;		/* window size */
static int xscale, yscale;
static int sx, sy;
#ifdef WANT_SDL
static SDL_Window *window;
static SDL_Renderer *renderer;
static int vio_win_id = -1;
#else /* !WANT_SDL */
static Display *display;
static Window window;
static int screen;
static GC gc;
static XWindowAttributes wa;
static Pixmap pixmap;
static Colormap colormap;
static XColor black, bg, fg;
static char black_color[] = "#000000";	/* black */
static XEvent event;
static KeySym key;
static char text[10];
#endif /* !WANT_SDL */
       uint8_t bg_color[3] = {48, 48, 48};	/* default background color */
       uint8_t fg_color[3] = {255, 255, 255};	/* default foreground color */

/* VIO stuff */
static int state;			/* state on/off for refresh thread */
static int mode;		/* video mode written to command port memory */
static int modebuf;			/* and double buffer for it */
static int vmode, res, inv;		/* video mode, resolution & inverse */
int imsai_kbd_status, imsai_kbd_data;	/* keyboard status & data */

#if !defined(WANT_SDL) || defined(HAS_NETSERVER)
/* UNIX stuff */
static pthread_t thread;
#endif

/* create the SDL2 or X11 window for VIO display */
static void open_display(void)
{
	xsize = 560 + (XOFF * 2);
	ysize = (240 * slf) + (YOFF * 2);

#ifdef WANT_SDL
	window = SDL_CreateWindow("IMSAI VIO",
				  SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED,
				  xsize, ysize, 0);
	renderer = SDL_CreateRenderer(window, -1, (SDL_RENDERER_ACCELERATED |
						   SDL_RENDERER_PRESENTVSYNC));

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
#else /* !WANT_SDL */
	Window rootwindow;
	XSizeHints *size_hints = XAllocSizeHints();
	Atom wm_delete_window;
	char buf[8];

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
	sprintf(buf, "#%02X%02X%02X", bg_color[0], bg_color[1], bg_color[2]);
	XParseColor(display, colormap, buf, &bg);
	XAllocColor(display, colormap, &bg);
	sprintf(buf, "#%02X%02X%02X", fg_color[0], fg_color[1], fg_color[2]);
	XParseColor(display, colormap, buf, &fg);
	XAllocColor(display, colormap, &fg);

	XMapWindow(display, window);
	XSync(display, True);
	XUnlockDisplay(display);
#endif /* !WANT_SDL */
}

/* close the SDL2 or X11 window for VIO display */
static void close_display(void)
{
#ifdef WANT_SDL
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	SDL_DestroyWindow(window);
	window = NULL;
#else
	XLockDisplay(display);
	XFreePixmap(display, pixmap);
	XFreeGC(display, gc);
	XUnlockDisplay(display);
	XCloseDisplay(display);
#endif
}

#if !defined(WANT_SDL) || defined(HAS_NETSERVER)
static void kill_thread(void)
{
	if (thread != 0) {
		sleep_for_ms(50);	/* wait a bit for thread to stop */
		pthread_cancel(thread);
		pthread_join(thread, NULL);
		thread = 0;
	}
}
#endif

/* shutdown VIO thread and window */
void imsai_vio_off(void)
{
	state = 0;		/* tell web refresh thread to stop */

#ifdef WANT_SDL
#ifdef HAS_NETSERVER
	if (!n_flag) {
#endif
		if (vio_win_id >= 0) {
			simsdl_destroy(vio_win_id);
			vio_win_id = -1;
		}
#ifdef HAS_NETSERVER
	} else
		kill_thread();
#endif
#else /* !WANT_SDL */
	kill_thread();
	if (display != NULL)
		close_display();
#endif /* !WANT_SDL */
}

#ifdef WANT_SDL

static inline void set_fg_color(void)
{
	SDL_SetRenderDrawColor(renderer,
			       fg_color[0], fg_color[1], fg_color[2],
			       SDL_ALPHA_OPAQUE);
}

static inline void set_bg_color(void)
{
	SDL_SetRenderDrawColor(renderer,
			       bg_color[0], bg_color[1], bg_color[2],
			       SDL_ALPHA_OPAQUE);
}

static inline void draw_point(int x, int y)
{
	SDL_RenderDrawPoint(renderer, x, y);
}

#else /* !WANT_SDL */

static inline void set_fg_color(void)
{
	XSetForeground(display, gc, fg.pixel);
}

static inline void set_bg_color(void)
{
	XSetForeground(display, gc, bg.pixel);
}

static inline void draw_point(int x, int y)
{
	XDrawPoint(display, pixmap, gc, x, y);
}

#endif /* !WANT_SDL */

/* display characters 80-FF from bits 0-6, bit 7 = inverse video */
static void dc1(BYTE c)
{
	register int x, y;
	int cinv = (c & 128) ? 1 : 0;

	for (x = 0; x < 7; x++) {
		for (y = 0; y < 10; y++) {
			if (charset[(c << 1) & 0xff][y][x] == 1) {
				if ((cinv ^ inv) == 0)
					set_fg_color();
				else
					set_bg_color();
			} else {
				if ((cinv ^ inv) == 0)
					set_bg_color();
				else
					set_fg_color();
			}
			draw_point(sx + (x * xscale), sy + (y * yscale * slf));
			if (res & 1)
				draw_point(sx + (x * xscale) + 1,
					   sy + (y * yscale * slf));
			if (res & 2)
				draw_point(sx + (x * xscale),
					   sy + (y * yscale * slf) + (1 * slf));
			if ((res & 3) == 3)
				draw_point(sx + (x * xscale) + 1,
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
					set_fg_color();
				else
					set_bg_color();
			} else {
				if ((cinv ^ inv) == 0)
					set_bg_color();
				else
					set_fg_color();
			}
			draw_point(sx + (x * xscale), sy + (y * yscale * slf));
			if (res & 1)
				draw_point(sx + (x * xscale) + 1,
					   sy + (y * yscale * slf));
			if (res & 2)
				draw_point(sx + (x * xscale),
					   sy + (y * yscale * slf) + (1 * slf));
			if ((res & 3) == 3)
				draw_point(sx + (x * xscale) + 1,
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
					set_fg_color();
				else
					set_bg_color();
			} else {
				if (inv == 0)
					set_bg_color();
				else
					set_fg_color();
			}
			draw_point(sx + (x * xscale), sy + (y * yscale * slf));
			if (res & 1)
				draw_point(sx + (x * xscale) + 1,
					   sy + (y * yscale * slf));
			if (res & 2)
				draw_point(sx + (x * xscale),
					   sy + (y * yscale * slf) + (1 * slf));
			if ((res & 3) == 3)
				draw_point(sx + (x * xscale) + 1,
					   sy + (y * yscale * slf) + (1 * slf));
		}
	}
}

#ifdef WANT_SDL

/*
 * Process a SDL2 event, we are only interested in keyboard input.
 * Note that I'm using the event queue as typeahead buffer, saves to
 * implement one self.
 */
static void event_handler(SDL_Event *event)
{
	/* if there is a keyboard event get it and convert to ASCII */
	switch (event->type) {
	case SDL_WINDOWEVENT:
		if (event->window.windowID == SDL_GetWindowID(window)) {
			switch (event->window.event) {
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				SDL_StartTextInput();
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				SDL_StopTextInput();
				break;
			default:
				break;
			}
		}
		break;
	case SDL_TEXTINPUT:
		if (event->text.windowID == SDL_GetWindowID(window)) {
			/* if the last character wasn't processed already do nothing */
			/* keep event in queue until the CPU emulation got current one */
			if (imsai_kbd_status != 0)
				return;

			imsai_kbd_data = event->text.text[0];
			imsai_kbd_status = 0;
		}
		break;
	case SDL_KEYDOWN:
		if (event->key.windowID == SDL_GetWindowID(window)) {
			/* if the last character wasn't processed already do nothing */
			/* keep event in queue until the CPU emulation got current one */
			if (imsai_kbd_status != 0)
				return;

			if (!(event->key.keysym.sym & SDLK_SCANCODE_MASK) &&
			    ((event->key.keysym.mod & KMOD_CTRL) ||
			     (event->key.keysym.sym < 32))) {
				imsai_kbd_data = event->key.keysym.sym & 0x1f;
				imsai_kbd_status = 0;
			}
		}
		break;
	default:
		break;
	}
#ifdef HAS_NETSERVER
	if (n_flag) {
		int res = net_device_get(DEV_VIO);
		if (res >= 0) {
			imsai_kbd_data =  res;
			imsai_kbd_status = 2;
		}
	}
#endif
}

#else /* !WANT_SDL */

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
	if (display != NULL && XEventsQueued(display, QueuedAlready) > 0) {
		XNextEvent(display, &event);
		if ((event.type == KeyPress) &&
		    XLookupString(&event.xkey, text, 1, &key, 0) == 1) {
			imsai_kbd_data = text[0];
			imsai_kbd_status = 2;
		}
	}
#ifdef HAS_NETSERVER
	if (n_flag) {
		int res = net_device_get(DEV_VIO);
		if (res >= 0) {
			imsai_kbd_data =  res;
			imsai_kbd_status = 2;
		}
	}
#endif
}

#endif /* !WANT_SDL */

/* refresh the display buffer dependent on video mode */
static void refresh(void)
{
	static int cols, rows;
	static BYTE c;
	register int x, y;

	sx = XOFF;
	sy = YOFF;

	mode = getmem(0xf7ff);
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
#ifdef WANT_SDL
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
#else
		event_handler();
		XSetForeground(display, gc, black.pixel);
		XFillRectangle(display, pixmap, gc, 0, 0, xsize, ysize);
#endif
		break;

	case 1: /* Video mode 1: display character codes 80-FF */
		for (y = 0; y < rows; y++) {
			sx = XOFF;
#ifndef WANT_SDL
			event_handler();
#endif
			for (x = 0; x < cols; x++) {
				c = getmem(0xf000 + (y * cols) + x);
				dc1(c);
				sx += (res & 1) ? 14 : 7;
			}
			sy += (res & 2) ? 20 * slf : 10 * slf;
		}
		break;

	case 2:	/* Video mode 2: display character codes 00-7F */
		for (y = 0; y < rows; y++) {
			sx = XOFF;
#ifndef WANT_SDL
			event_handler();
#endif
			for (x = 0; x < cols; x++) {
				c = getmem(0xf000 + (y * cols) + x);
				dc2(c);
				sx += (res & 1) ? 14 : 7;
			}
			sy += (res & 2) ? 20 * slf : 10 * slf;
		}
		break;

	case 3:	/* Video mode 3: display character codes 00-FF */
		for (y = 0; y < rows; y++) {
			sx = XOFF;
#ifndef WANT_SDL
			event_handler();
#endif
			for (x = 0; x < cols; x++) {
				c = getmem(0xf000 + (y * cols) + x);
				dc3(c);
				sx += (res & 1) ? 14 : 7;
			}
			sy += (res & 2) ? 20 * slf : 10 * slf;
		}
		break;
	}
}

#ifdef HAS_NETSERVER
static uint8_t dblbuf[2048];

static struct {
	uint16_t addr;
	union {
		uint16_t len;
		uint16_t mode;
	};
	uint8_t buf[2048];
} msg;

static void ws_refresh(void)
{
	static int cols, rows;

	mode = getmem(0xf7ff);
	if (mode != modebuf) {
		modebuf = mode;
		memset(dblbuf, 0, 2048);

		res = mode & 3;

		if (res & 1) {
			cols = 40;
		} else {
			cols = 80;
		}

		if (res & 2) {
			rows = 12;
		} else {
			rows = 24;
		}

		msg.mode = mode;
		msg.addr = 0xf7ff;
		net_device_send(DEV_VIO, (char *) &msg, 4);
		LOGD(__func__, "MODE change");
	}

#ifndef WANT_SDL
	event_handler();
#endif

	int len = rows * cols;
	int addr;
	int i, n, x;
	bool cont;
	uint8_t val;

	for (i = 0; i < len; i++) {
		addr = i;
		n = 0;
		cont = true;
		while (cont && (i < len)) {
			val = getmem(0xf000 + i);
			while ((val != dblbuf[i]) && (i < len)) {
				dblbuf[i++] = val;
				msg.buf[n++] = val;
				cont = false;
				val = getmem(0xf000 + i);
			}
			if (cont)
				break;
			x = 0;
#define LOOKAHEAD 4
			/* look-ahead up to 4 bytes for next change */
			while ((x < LOOKAHEAD) && !cont && (i < len)) {
				val = getmem(0xf000 + i++);
				msg.buf[n++] = val;
				val = getmem(0xf000 + i);
				if ((i < len) && (val != dblbuf[i])) {
					cont = true;
				}
				x++;
			}
			if (!cont) {
				n -= x;
			}
		}
		if (n) {
			msg.addr = 0xf000 + addr;
			msg.len = n;
			net_device_send(DEV_VIO, (char *) &msg, msg.len + 4);
			LOGD(__func__, "BUF update FROM %04X TO %04X",
			     msg.addr, msg.addr + msg.len);
		}
	}
}
#endif /* HAS_NETSERVER */

#ifdef WANT_SDL
/* function for updating the display */
static void update_display(bool tick)
{
	UNUSED(tick);

	/* update display window */
	refresh();
	SDL_RenderPresent(renderer);
}

static win_funcs_t vio_funcs = {
	open_display,
	close_display,
	event_handler,
	update_display
};
#endif /* !WANT_SDL */

#if !defined(WANT_SDL) || defined(HAS_NETSERVER)
/* thread for updating the X11 display or web server */
static void *update_thread(void *arg)
{
	uint64_t t1, t2;
	int tdiff;

	UNUSED(arg);

	t1 = get_clock_us();

	while (state) {
#ifdef HAS_NETSERVER
		if (!n_flag) {
#endif
#ifndef WANT_SDL
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
#endif
#ifdef HAS_NETSERVER
		} else
			ws_refresh();
#endif

		/* sleep rest to 33ms so that we get 30 fps */
		t2 = get_clock_us();
		tdiff = t2 - t1;
		if ((tdiff > 0) && (tdiff < 33000))
			sleep_for_ms(33 - (tdiff / 1000));

		t1 = get_clock_us();
	}

	pthread_exit(NULL);
}
#endif /* !WANT_SDL || HAS_NETSERVER */

/* create the SDL window and start display refresh thread */
void imsai_vio_init(void)
{
#ifdef HAS_NETSERVER
	if (!n_flag) {
#endif
#ifdef WANT_SDL
		if (vio_win_id < 0)
			vio_win_id = simsdl_create(&vio_funcs);
#else
		if (display == NULL)
			open_display();
#endif
#ifdef HAS_NETSERVER
	}
#endif

	state = 1;
	modebuf = -1;
	putmem(0xf7ff, 0x00);

#if defined(WANT_SDL) && defined(HAS_NETSERVER)
	if (n_flag) {
#endif
#if !defined(WANT_SDL) || defined(HAS_NETSERVER)
		if (pthread_create(&thread, NULL, update_thread, (void *) NULL)) {
			LOGE(TAG, "can't create thread");
			exit(EXIT_FAILURE);
		}
#endif
#if defined(WANT_SDL) && defined(HAS_NETSERVER)
	}
#endif
}
