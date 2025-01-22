/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2015-2019 by Udo Munk
 * Copyright (C) 2025 by Thomas Eberhardt
 */

/*
 *	This module contains an introspection panel to view various
 *	status information of the simulator.
 */

#include <string.h>
#ifdef WANT_SDL
#include <SDL.h>
#else
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pthread.h>
#endif

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"
#include "simpanel.h"
#include "simport.h"
#ifdef WANT_SDL
#include "simsdl.h"
#endif

/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"
static const char *TAG = "panel";

/* 888 RGB colors */
#define C_BLACK		0x00000000
#define C_RED		0x00ff0000
#define C_GREEN		0x0000ff00
#define C_BLUE		0x000000ff
#define C_CYAN		0x0000ffff
#define C_MAGENTA	0x00ff00ff
#define C_YELLOW	0x00ffff00
#define C_WHITE		0x00ffffff
#define C_DKRED		0x00800000
#define C_DKGREEN	0x00008000
#define C_DKBLUE	0x00000080
#define C_DKCYAN	0x00008080
#define C_DKMAGENTA	0x00800080
#define C_DKYELLOW	0x00808000
#define C_GRAY		0x00808080
#define C_ORANGE	0x00ffa500
#define C_WHEAT		0x00f5deb3

/*
 *	Font type. Depth is ignored and assumed to be 1.
 */
typedef const struct font {
	const uint8_t *bits;
	const unsigned depth;
	const unsigned width;
	const unsigned height;
	const unsigned stride;
} font_t;

/*
 *	Grid type for drawing text with character based coordinates.
 */
typedef struct draw_grid {
	const font_t *font;
	unsigned xoff;
	unsigned yoff;
	unsigned spc;
	unsigned cwidth;
	unsigned cheight;
	unsigned cols;
	unsigned rows;
} draw_grid_t;

/* include Terminus bitmap fonts */
#include "fonts/font12.h"
#include "fonts/font14.h"
#include "fonts/font16.h"
#include "fonts/font18.h"
#include "fonts/font20.h"
#include "fonts/font22.h"
#include "fonts/font24.h"
#include "fonts/font28.h"
#include "fonts/font32.h"

/* SDL2/X11 stuff */
static unsigned xsize, ysize;
static uint32_t *pixels;
static int pitch;
#ifdef WANT_SDL
static int panel_win_id = -1;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
#else
static Display *display;
static Visual *visual;
static Window window;
static int screen;
static GC gc;
static XImage *ximage;
static Colormap colormap;
static XEvent event;
#endif

#ifndef WANT_SDL
/* UNIX stuff */
static pthread_t thread;
#endif

/*
 * Create the SDL2 or X11 window for panel display
 */
static void open_display(void)
{
	xsize = 240;
	ysize = 135;

#ifdef WANT_SDL
	window = SDL_CreateWindow("Z80pack",
				  SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED,
				  xsize, ysize, 0);
	if (window == NULL) {
		LOGW(TAG, "can't create window: %s", SDL_GetError());
		return;
	}
	renderer = SDL_CreateRenderer(window, -1, (SDL_RENDERER_ACCELERATED |
						   SDL_RENDERER_PRESENTVSYNC));
	if (renderer == NULL) {
		LOGW(TAG, "can't create renderer: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		window = NULL;
		return;
	}
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888,
				    SDL_TEXTUREACCESS_STREAMING, xsize, ysize);
	if (texture == NULL) {
		LOGW(TAG, "can't create texture: %s", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
		SDL_DestroyWindow(window);
		window = NULL;
		return;
	}
#else /* !WANT_SDL */
	Window rootwindow;
	XSetWindowAttributes swa;
	XSizeHints *size_hints = XAllocSizeHints();
	Atom wm_delete_window;
	XVisualInfo vinfo;

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		LOGW(TAG, "can't open display %s", getenv("DISPLAY"));
		return;
	}
	XLockDisplay(display);
	screen = DefaultScreen(display);
	if (!XMatchVisualInfo(display, screen, 24, TrueColor, &vinfo)) {
		LOGW(TAG, "couldn't find a 24-bit TrueColor visual");
		XUnlockDisplay(display);
		XCloseDisplay(display);
		display = NULL;
		return;
	}
	rootwindow = RootWindow(display, vinfo.screen);
	visual = vinfo.visual;
	colormap = XCreateColormap(display, rootwindow, visual, AllocNone);
	swa.border_pixel = 0;
	swa.colormap = colormap;
	swa.event_mask = ButtonPressMask | ButtonReleaseMask;
	window = XCreateWindow(display, rootwindow, 0, 0, xsize, ysize,
			       1, vinfo.depth, InputOutput, visual,
			       CWBorderPixel | CWColormap | CWEventMask, &swa);
	XStoreName(display, window, "Z80pack");
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
	gc = XCreateGC(display, window, 0, NULL);
	pixels = (uint32_t *) malloc (xsize * ysize * sizeof(uint32_t));
	ximage = XCreateImage(display, visual, vinfo.depth, ZPixmap, 0,
			      (char *) pixels, xsize, ysize, 32, 0);
	/* force little-endian pixels, Xlib will convert if necessary */
	ximage->byte_order = LSBFirst;
	pitch = ximage->bytes_per_line / 4;

	XMapWindow(display, window);
	XUnlockDisplay(display);
#endif /* !WANT_SDL */
}

/*
 * Close the SDL2 or X11 window for panel display
 */
static void close_display(void)
{
#ifdef WANT_SDL
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
	if (renderer != NULL) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}
	if (window != NULL) {
		SDL_DestroyWindow(window);
		window = NULL;
	}
#else
	if (display != NULL) {
		XLockDisplay(display);
		free(pixels);
		ximage->data = NULL;
		XDestroyImage(ximage);
		XFreeGC(display, gc);
		XDestroyWindow(display, window);
		XFreeColormap(display, colormap);
		XUnlockDisplay(display);
		XCloseDisplay(display);
		display = NULL;
	}
#endif
}

#ifdef WANT_SDL

/*
 * Process a SDL event
 */
static void process_event(SDL_Event *event)
{
	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN:
		break;
	case SDL_MOUSEBUTTONUP:
		break;
	default:
		break;
	}
}

#else /* !WANT_SDL */

/*
 * Process the X11 event queue
 */
static inline void process_events(void)
{
	while (XPending(display)) {
		XNextEvent(display, &event);
		switch (event.type) {
		case ButtonPress:
			break;
		case ButtonRelease:
			break;
		default:
			break;
		}
	}
}

#endif /* !WANT_SDL */

#define DRAW_DEBUG

/*
 *	Fill the pixmap with the specified color.
 */
static inline void draw_clear(const uint32_t color)
{
	uint32_t *p = pixels;
	unsigned x, y;

#ifdef DRAW_DEBUG
	if (pixels == NULL) {
		fprintf(stderr, "%s: pixels texture is NULL\n", __func__);
		return;
	}
#endif
	for (x = 0; x < xsize; x++)
		*p++ = color;
	for (y = 1; y < ysize; y++) {
		memcpy(p, pixels, pitch * 4);
		p += pitch;
	}
}

/*
 *	Draw a pixel in the specified color.
 */
static inline void draw_pixel(const unsigned x, const unsigned y,
			      const uint32_t color)
{
#ifdef DRAW_DEBUG
	if (pixels == NULL) {
		fprintf(stderr, "%s: pixels texture is NULL\n", __func__);
		return;
	}
	if (x >= xsize || y >= ysize) {
		fprintf(stderr, "%s: coord (%d,%d) is outside (0,0)-(%d,%d)\n",
			__func__, x, y, xsize - 1, ysize - 1);
		return;
	}
#endif
	*(pixels + y * pitch + x) = color;
}

/*
 *	Draw a character in the specfied font and colors.
 */
static inline void draw_char(const unsigned x, const unsigned y, const char c,
			     const font_t *font, const uint32_t fgc,
			     const uint32_t bgc)
{
	const unsigned off = (c & 0x7f) * font->width;
	const uint8_t *p0 = font->bits + (off >> 3), *p;
	const uint8_t m0 = 0x80 >> (off & 7);
	uint8_t m;
	uint32_t *q0, *q;
	unsigned i, j;

#ifdef DRAW_DEBUG
	if (pixels == NULL) {
		fprintf(stderr, "%s: pixels texture is NULL\n", __func__);
		return;
	}
	if (font == NULL) {
		fprintf(stderr, "%s: font is NULL\n", __func__);
		return;
	}
	if (x >= xsize || y >= ysize || x + font->width > xsize ||
	    y + font->height > ysize) {
		fprintf(stderr, "%s: char '%c' at (%d,%d)-(%d,%d) is "
			"outside (0,0)-(%d,%d)\n", __func__, c, x, y,
			x + font->width - 1, y + font->height - 1,
			xsize - 1, ysize - 1);
		return;
	}
#endif
	q0 = pixels + y * pitch + x;
	for (j = font->height; j > 0; j--) {
		m = m0;
		p = p0;
		q = q0;
		for (i = font->width; i > 0; i--) {
			if (*p & m)
				*q = fgc;
			else
				*q = bgc;
			if ((m >>= 1) == 0) {
				m = 0x80;
				p++;
			}
			q++;
		}
		p0 += font->stride;
		q0 += pitch;
	}
}

/*
 *	Draw a horizontal line in the specified color.
 */
static inline void draw_hline(const unsigned x, const unsigned y, unsigned w,
			      const uint32_t col)
{
	uint32_t *p;

#ifdef DRAW_DEBUG
	if (pixels == NULL) {
		fprintf(stderr, "%s: pixels texture is NULL\n", __func__);
		return;
	}
	if (x >= xsize || y >= ysize || x + w > xsize) {
		fprintf(stderr, "%s: line (%d,%d)-(%d,%d) is outside "
			"(0,0)-(%d,%d)\n", __func__, x, y, x + w - 1, y,
			xsize - 1, ysize - 1);
		return;
	}
#endif
	p = pixels + y * pitch + x;
	while (w--)
		*p++ = col;
}

/*
 *	Draw a vertical line in the specified color.
 */
static inline void draw_vline(const unsigned x, const unsigned y, unsigned h,
			      const uint32_t col)
{
	uint32_t *p;

#ifdef DRAW_DEBUG
	if (pixels == NULL) {
		fprintf(stderr, "%s: pixels texture is NULL\n", __func__);
		return;
	}
	if (x >= xsize || y >= ysize || y + h > ysize) {
		fprintf(stderr, "%s: line (%d,%d)-(%d,%d) is outside "
			"(0,0)-(%d,%d)\n", __func__, x, y, x, y + h - 1,
			xsize - 1, ysize - 1);
		return;
	}
#endif
	p = pixels + y * pitch + x;
	while (h--) {
		*p = col;
		p += pitch;
	}
}

/*
 *	Setup a text grid defined by font and spacing.
 *	If col < 0 then use the entire pixels texture width.
 *	If row < 0 then use the entire pixels texture height.
 */
static inline void draw_setup_grid(draw_grid_t *grid, const unsigned xoff,
				   const unsigned yoff, const int cols,
				   const int rows, const font_t *font,
				   const unsigned spc)
{
#ifdef DRAW_DEBUG
	if (pixels == NULL) {
		fprintf(stderr, "%s: pixels texture is NULL\n", __func__);
		return;
	}
	if (grid == NULL) {
		fprintf(stderr, "%s: grid is NULL\n", __func__);
		return;
	}
	if (font == NULL) {
		fprintf(stderr, "%s: font is NULL\n", __func__);
		return;
	}
	if (cols == 0) {
		fprintf(stderr," %s: number of columns is zero\n", __func__);
		return;
	}
	if (cols >= 0 && (unsigned) cols > (xsize - xoff) / font->width) {
		fprintf(stderr," %s: number of columns %d is too large\n",
			__func__, cols);
		return;
	}
	if (rows == 0) {
		fprintf(stderr," %s: number of rows is zero\n", __func__);
		return;
	}
	if (rows >= 0 && (unsigned) rows > ((ysize - yoff + spc) /
					    (font->height + spc))) {
		fprintf(stderr," %s: number of rows %d is too large\n",
			__func__, rows);
		return;
	}
#endif
	grid->font = font;
	grid->xoff = xoff;
	grid->yoff = yoff;
	grid->spc = spc;
	grid->cwidth = font->width;
	grid->cheight = font->height + spc;
	if (cols < 0)
		grid->cols = (xsize - xoff) / grid->cwidth;
	else
		grid->cols = cols;
	if (rows < 0)
		grid->rows = (ysize - yoff + spc) / grid->cheight;
	else
		grid->rows = rows;
}

/*
 *	Draw a character using grid coordinates in the specified color.
 */
static inline void draw_grid_char(const unsigned x, const unsigned y,
				  const char c, const draw_grid_t *grid,
				  const uint32_t fgc, const uint32_t bgc)
{
#ifdef DRAW_DEBUG
	if (pixels == NULL) {
		fprintf(stderr, "%s: pixels texture is NULL\n", __func__);
		return;
	}
	if (grid == NULL) {
		fprintf(stderr, "%s: grid is NULL\n", __func__);
		return;
	}
#endif
	draw_char(x * grid->cwidth + grid->xoff,
		  y * grid->cheight + grid->yoff,
		  c, grid->font, fgc, bgc);
}

/*
 *	Draw a horizontal grid line in the middle of the spacing
 *	above the y grid coordinate specified.
 */
static inline void draw_grid_hline(unsigned x, unsigned y, unsigned w,
				   const draw_grid_t *grid, const uint32_t col)
{
#ifdef DRAW_DEBUG
	if (pixels == NULL) {
		fprintf(stderr, "%s: pixels texture is NULL\n", __func__);
		return;
	}
	if (grid == NULL) {
		fprintf(stderr, "%s: grid is NULL\n", __func__);
		return;
	}
#endif
	if (w) {
		x = x * grid->cwidth;
		if (y)
			y = y * grid->cheight - (grid->spc + 1) / 2;
		w = w * grid->cwidth;
		draw_hline(x + grid->xoff, y + grid->yoff, w, col);
	}
}

/*
 *	Draw a vertical grid line in the middle of the x grid coordinate
 *	specified.
 */
static inline void draw_grid_vline(unsigned x, unsigned y, unsigned h,
				   const draw_grid_t *grid, const uint32_t col)
{
#ifdef DRAW_DEBUG
	if (pixels == NULL) {
		fprintf(stderr, "%s: pixels texture is NULL\n", __func__);
		return;
	}
	if (grid == NULL) {
		fprintf(stderr, "%s: grid is NULL\n", __func__);
		return;
	}
#endif
	unsigned hadj = 0;

	if (h) {
		x = x * grid->cwidth + (grid->cwidth + 1) / 2;
		if (y + h < grid->rows)
			hadj += grid->spc / 2 + 1;
		if (y) {
			y = y * grid->cheight - (grid->spc + 1) / 2;
			hadj += (grid->spc + 1) / 2;
		}
		h = h * grid->cheight - grid->spc + hadj;
		draw_vline(x + grid->xoff, y + grid->yoff, h, col);
	}
}

/*
 *	Draw a 10x10 LED circular bracket.
 */
static inline void draw_led_bracket(const unsigned x, const unsigned y)
{
	draw_hline(x + 2, y, 6, C_GRAY);
	draw_pixel(x + 1, y + 1, C_GRAY);
	draw_pixel(x + 8, y + 1, C_GRAY);
	draw_vline(x, y + 2, 6, C_GRAY);
	draw_vline(x + 9, y + 2, 6, C_GRAY);
	draw_pixel(x + 1, y + 8, C_GRAY);
	draw_pixel(x + 8, y + 8, C_GRAY);
	draw_hline(x + 2, y + 9, 6, C_GRAY);
}

/*
 *	Draw a LED inside a 10x10 circular bracket.
 */
static inline void draw_led(const unsigned x, const unsigned y,
			    const uint32_t col)
{
	int i;

	for (i = 1; i < 9; i++) {
		if (i == 1 || i == 8)
			draw_hline(x + 2, y + i, 6, col);
		else
			draw_hline(x + 1, y + i, 8, col);
	}
}

/*
 *	CPU status displays:
 *
 *	Z80 CPU using font20 (10 x 20 pixels):
 *
 *	  01234567890123456789012
 *	0 A  xx   BC xxxx DE xxxx
 *	1 HL xxxx SP xxxx PC xxxx
 *	2 IX xxxx IY xxxx AF'xxxx
 *	3 BC'xxxx DE'xxxx HL'xxxx
 *	4 F  SZHPNC  IF12 IR xxxx
 *	Model x.x   o xx.xx°C
 *
 *	8080 CPU using font28 (14 x 28 pixels):
 *
 *	  0123456789012345
 *	0 A  xx    BC xxxx
 *	1 DE xxxx  HL xxxx
 *	2 SP xxxx  PC xxxx
 *	3 F  SZHPC    IF 1
 *	Model x.x   o xx.xx°C
 */

typedef struct reg {
	uint8_t x;
	uint8_t y;
	enum { RB, RW, RF, RI, RA, RR } type;
	const char *l;
	union {
		struct {
			const BYTE *p;
		} b;
		struct {
			const WORD *p;
		} w;
		struct {
			char c;
			uint8_t m;
		} f;
	};
} reg_t;

#ifndef EXCLUDE_Z80

#define XOFF20	5	/* x pixel offset of text grid for font20 */
#define YOFF20	0	/* y pixel offset of text grid for font20 */
#define SPC20	3	/* vertical text spacing for font20 */

static const reg_t regs_z80[] = {
	{  4, 0, RB, "A",    .b.p = &A },
	{ 12, 0, RB, "BC",   .b.p = &B },
	{ 14, 0, RB, NULL,   .b.p = &C },
	{ 20, 0, RB, "DE",   .b.p = &D },
	{ 22, 0, RB, NULL,   .b.p = &E },
	{  4, 1, RB, "HL",   .b.p = &H },
	{  6, 1, RB, NULL,   .b.p = &L },
	{ 14, 1, RW, "SP",   .w.p = &SP },
	{ 22, 1, RW, "PC",   .w.p = &PC },
	{  6, 2, RW, "IX",   .w.p = &IX },
	{ 14, 2, RW, "IY",   .w.p = &IY },
	{ 20, 2, RB, "AF\'", .b.p = &A_ },
	{ 22, 2, RA, NULL,   .b.p = NULL },
	{  4, 3, RB, "BC\'", .b.p = &B_ },
	{  6, 3, RB, NULL,   .b.p = &C_ },
	{ 12, 3, RB, "DE\'", .b.p = &D_ },
	{ 14, 3, RB, NULL,   .b.p = &E_ },
	{ 20, 3, RB, "HL\'", .b.p = &H_ },
	{ 22, 3, RB, NULL,   .b.p = &L_ },
	{  3, 4, RF, NULL,   .f.c = 'S', .f.m = S_FLAG },
	{  4, 4, RF, "F",    .f.c = 'Z', .f.m = Z_FLAG },
	{  5, 4, RF, NULL,   .f.c = 'H', .f.m = H_FLAG },
	{  6, 4, RF, NULL,   .f.c = 'P', .f.m = P_FLAG },
	{  7, 4, RF, NULL,   .f.c = 'N', .f.m = N_FLAG },
	{  8, 4, RF, NULL,   .f.c = 'C', .f.m = C_FLAG },
	{ 13, 4, RI, NULL,   .f.c = '1', .f.m = 1 },
	{ 14, 4, RI, "IF",   .f.c = '2', .f.m = 2 },
	{ 20, 4, RB, "IR",   .b.p = &I },
	{ 22, 4, RR, NULL,   .b.p = NULL }
};
static const int num_regs_z80 = sizeof(regs_z80) / sizeof(reg_t);

#endif /* !EXCLUDE_Z80 */

#ifndef EXCLUDE_I8080

#define XOFF28	8	/* x pixel offset of text grid for font28 */
#define YOFF28	0	/* y pixel offset of text grid for font28 */
#define SPC28	1	/* vertical text spacing for font28 */

static const reg_t regs_8080[] = {
	{  4, 0, RB, "A",  .b.p = &A },
	{ 13, 0, RB, "BC", .b.p = &B },
	{ 15, 0, RB, NULL, .b.p = &C },
	{  4, 1, RB, "DE", .b.p = &D },
	{  6, 1, RB, NULL, .b.p = &E },
	{ 13, 1, RB, "HL", .b.p = &H },
	{ 15, 1, RB, NULL, .b.p = &L },
	{  6, 2, RW, "SP", .w.p = &SP },
	{ 15, 2, RW, "PC", .w.p = &PC },
	{  3, 3, RF, NULL, .f.c = 'S', .f.m = S_FLAG },
	{  4, 3, RF, "F",  .f.c = 'Z', .f.m = Z_FLAG },
	{  5, 3, RF, NULL, .f.c = 'H', .f.m = H_FLAG },
	{  6, 3, RF, NULL, .f.c = 'P', .f.m = P_FLAG },
	{  7, 3, RF, NULL, .f.c = 'C', .f.m = C_FLAG },
	{ 15, 3, RI, "IF", .f.c = '1', .f.m = 3 }
};
static const int num_regs_8080 = sizeof(regs_8080) / sizeof(reg_t);

#endif /* !EXCLUDE_I8080 */

/*
 * Refresh the display buffer
 */
static void refresh(bool tick)
{
	char c;
	int i, j, f, digit, n = 0;
	bool onlyz;
	unsigned x, y;
	WORD w;
	const char *s;
	const reg_t *rp = NULL;
	draw_grid_t grid = { };
	int cpu_type = cpu;
	static int freq;

	/* use cpu_type in the rest of this function, since cpu can change */

#ifndef EXCLUDE_Z80
	if (cpu_type == Z80) {
		rp = regs_z80;
		n = num_regs_z80;
	}
#endif
#ifndef EXCLUDE_I8080
	if (cpu_type == I8080) {
		rp = regs_8080;
		n = num_regs_8080;
	}
#endif

	draw_clear(C_DKBLUE);

	/* setup text grid and draw grid lines */
#ifndef EXCLUDE_Z80
	if (cpu_type == Z80) {
		draw_setup_grid(&grid, XOFF20, YOFF20, -1, 5, &font20,
				SPC20);

		/* draw vertical grid lines */
		draw_grid_vline(7, 0, 4, &grid, C_DKYELLOW);
		draw_grid_vline(10, 4, 1, &grid, C_DKYELLOW);
		draw_grid_vline(15, 0, 5, &grid, C_DKYELLOW);
		/* draw horizontal grid lines */
		for (i = 1; i < 5; i++)
			draw_grid_hline(0, i, grid.cols, &grid,
					C_DKYELLOW);
	}
#endif
#ifndef EXCLUDE_I8080
	if (cpu_type == I8080) {
		draw_setup_grid(&grid, XOFF28, YOFF28, -1, 4, &font28,
				SPC28);

		/* draw vertical grid line */
		draw_grid_vline(8, 0, 4, &grid, C_DKYELLOW);
		/* draw horizontal grid lines */
		for (i = 1; i < 4; i++)
			draw_grid_hline(0, i, grid.cols, &grid,
					C_DKYELLOW);
	}
#endif
	/* draw register labels & contents */
	for (i = 0; i < n; rp++, i++) {
		if ((s = rp->l) != NULL) {
			x = rp->x - (rp->type == RW ? 6 : 4);
			if (rp->type == RI)
				x++;
			while (*s)
				draw_grid_char(x++, rp->y, *s++, &grid,
					       C_WHITE, C_DKBLUE);
		}
		switch (rp->type) {
		case RB: /* byte sized register */
			w = *(rp->b.p);
			j = 2;
			break;
		case RW: /* word sized register */
			w = *(rp->w.p);
			j = 4;
			break;
		case RF: /* flags */
			draw_grid_char(rp->x, rp->y, rp->f.c, &grid,
				       (F & rp->f.m) ? C_GREEN : C_RED,
				       C_DKBLUE);
			continue;
		case RI: /* interrupt register */
			draw_grid_char(rp->x, rp->y, rp->f.c, &grid,
				       (IFF & rp->f.m) == rp->f.m ?
				       C_GREEN : C_RED, C_DKBLUE);
			continue;
#ifndef EXCLUDE_Z80
		case RA: /* alternate flags (int) */
			w = F_;
			j = 2;
			break;
		case RR: /* refresh register */
			w = (R_ & 0x80) | (R & 0x7f);
			j = 2;
			break;
#endif
		default:
			continue;
		}
		x = rp->x;
		while (j--) {
			c = w & 0xf;
			c += (c < 10 ? '0' : 'A' - 10);
			draw_grid_char(x--, rp->y, c, &grid, C_GREEN,
				       C_DKBLUE);
			w >>= 4;
		}
	}

	y = ysize - font20.height;
	n = xsize / font20.width;
	x = (xsize - n * font20.width) / 2;

	/* draw product info */
	s = "Z80pack " RELEASE;
	for (i = 0; *s; i++)
		draw_char(i * font20.width + x, y, *s++, &font20,
			  C_ORANGE, C_DKBLUE);

	/* draw frequency label */
	draw_char((n - 6) * font20.width + x, y, '.', &font20,
		  C_ORANGE, C_DKBLUE);
	draw_char((n - 3) * font20.width + x, y, 'M', &font20,
		  C_ORANGE, C_DKBLUE);
	draw_char((n - 2) * font20.width + x, y, 'H', &font20,
		  C_ORANGE, C_DKBLUE);
	draw_char((n - 1) * font20.width + x, y, 'z', &font20,
		  C_ORANGE, C_DKBLUE);

	/* update frequency every second */
	if (tick && cpu_time)
		freq = (int) ((float) T_freq / (float) cpu_time * 100.0);
	f = freq;
	digit = 100000;
	onlyz = true;
	for (i = 0; i < 7; i++) {
		c = '0';
		while (f > digit) {
			f -= digit;
			c++;
		}
		if (onlyz && i < 3 && c == '0')
			c = ' ';
		else
			onlyz = false;
		draw_char((n - 10 + i) * font20.width + x, y,
			  c, &font20, C_ORANGE, C_DKBLUE);
		if (i < 6)
			digit /= 10;
		if (i == 3)
			i++; /* skip decimal point */
	}
}

#ifdef WANT_SDL

/* function for updating the display */
static void update_display(bool tick)
{
	SDL_LockTexture(texture, NULL, (void **) &pixels, &pitch);
	pitch /= 4;
	refresh(tick);
	SDL_UnlockTexture(texture);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

static win_funcs_t panel_funcs = {
	open_display,
	close_display,
	process_event,
	update_display
};

#else /* !WANT_SDL */

static void kill_thread(void)
{
	if (thread != 0) {
		sleep_for_ms(50);
		pthread_cancel(thread);
		pthread_join(thread, NULL);
		thread = 0;
	}
}

/* thread for updating the display */
static void *update_display(void *arg)
{
	uint64_t t1, t2, ttick;
	long tleft;
	bool tick = true;

	UNUSED(arg);

	t1 = get_clock_us();
	ttick = t + 1000000;

	while (true) {
		/* lock display, don't cancel thread while locked */
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		XLockDisplay(display);

		/* process X11 event queue */
		process_events();

		/* update display window */
		refresh(tick);
		XPutImage(display, window, gc, ximage, 0, 0, 0, 0,
			  xsize, ysize);
		XSync(display, False);

		/* unlock display, thread can be canceled again */
		XUnlockDisplay(display);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		t2 = get_clock_us();

		/* update seconds tick */
		if ((tick = (t2 >= ttick)))
			ttick = t2 + 1000000;

		/* sleep rest to 16667us so that we get 60 fps */
		tleft = 16667L - (long) (t2 - t1);
		if (tleft > 0)
			sleep_for_us(tleft);

		t1 = get_clock_us();
	}

	/* just in case it ever gets here */
	pthread_exit(NULL);
}

#endif /* !WANT_SDL */

void init_panel(void)
{
#ifdef WANT_SDL
	if (panel_win_id < 0)
		panel_win_id = simsdl_create(&panel_funcs);
#else
	if (display == NULL) {
		open_display();

		if (pthread_create(&thread, NULL, update_display, (void *) NULL)) {
			LOGE(TAG, "can't create thread");
			exit(EXIT_FAILURE);
		}
	}
#endif
}

void exit_panel(void)
{
#ifdef WANT_SDL
	if (panel_win_id >= 0) {
		simsdl_destroy(panel_win_id);
		panel_win_id = -1;
	}
#else /* !WANT_SDL */
	kill_thread();
	if (display != NULL)
		close_display();
#endif /* !WANT_SDL */
}
