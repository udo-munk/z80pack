// lpanel.h	lightpanel

/* Copyright (c) 2007-2008, John Kichury
   C conversion and SDL2 support is Copyright (c) 2024-2025, Thomas Eberhardt

   This software is freely distributable free of charge and without license fees with the
   following conditions:

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   JOHN KICHURY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The above copyright notice must be included in any copies of this software.

*/

#ifndef _LPANEL_DEFS
#define _LPANEL_DEFS

#include <stdbool.h>
#include <stdint.h>
#ifdef WANT_SDL
#include <SDL.h>
#include <SDL_opengl.h>
#else /* !WANT_SDL */
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif
#endif /* !WANT_SDL */

#define LP_MAX_LIGHT_GROUPS 10

// forward references

struct lpLight;
struct lpSwitch;
struct Lpanel;

enum obj_types { LP_NULL, LP_LED };
enum obj_subtypes { LP_SUBTYPE_NULL, LP_LED_3D, LP_LED_2D };
enum gfx_projection { LP_ORTHO, LP_PERSPECTIVE };

// light sampling datatypes

enum light_bindtypes {
	LBINDTYPE_NULL,
	LBINDTYPE_BIT, 		// data pointer points to a single word of bits
				// (bitnum is used as a bit number)
	LBINDTYPE_FLOATV	// data pointer points to an array
				// (bitnum is used as an index)
};

// view structure
//

typedef struct view {
	int	projection;	// ortho, perspective
	bool	do_depthtest;	// true = enable depth test
	float	rot[3],
		pan[3];
	double	aspect, fovy, znear, zfar;

	bool	redo_projections;
} view_t;

// object parm structure
// ---------------------

typedef struct lp_obj_parm {
	int	type,		// LP_LIGHT, LP_SWITCH etc.
		subtype,	// LP_LED
		group;

	float	pos[3],		// xyz position
		color[3],	// rgb color
		scale[3];
} lp_obj_parm_t;

typedef struct lp_light_group {
	int 	num_items,
		max_items,
		*list;
} lp_light_group_t;

#include "lp_gfx.h"
#include "lp_switch.h"

// lightpanel class
// ----------------

typedef void (*lp_quit_cbf_t)(void);

typedef struct Lpanel {
	// private variables
	int		num_lights,
			max_lights;

	struct lpLight	**lights;

	lp_light_group_t light_groups[LP_MAX_LIGHT_GROUPS];

	lpSwitch_t	**switches;

	int		num_switches,
			max_switches;

#ifdef WANT_SDL
	SDL_Window	*window;	// SDL window
	SDL_GLContext	cx;
#else /* !WANT_SDL */
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
	HINSTANCE	hInstance;
	HWND		hWnd;
	WNDCLASSEX	wc;
	HDC		hDC;
	HGLRC		hRC;
	PIXELFORMATDESCRIPTOR pfd;
#else
	Display		*dpy;		// Xwindows display
	Window		window;		// Xwindows window
	XVisualInfo	*vi;
	GLXContext	cx;
	Atom		wmDeleteMessage; // for processing window close event
#endif
#endif /* !WANT_SDL */

	view_t		view;

	lpElement_t	*curr_element;
	vertex_t	*curr_vertex;
	bool		envmap_detected;

	char		*config_root_path;	// pathname to configuration root (default is ./)

	lpTextures_t	textures;

	// public variables
	uint64_t	default_clock,
			*simclock,
			old_clock;
	int		clock_warp;
	bool		ignore_bind_errors;
	uint8_t		default_runflag,
			*runflag;
	lpSwitch_t	*mom_switch_pressed;

	float		lightcolor[3],	// default light color
			lightsize[3];	// default light size

	lp_quit_cbf_t	quit_callbackfunc;

	int		num_objects,
			max_objects,
			num_alpha_objects,
			max_alpha_objects;

	lpObject_t	**objects,
			*curr_object,
			**alpha_objects;

	lpBBox_t	bbox;

	GLint		viewport[4];
	int		window_xsize,
			window_ysize;

	// dev vars

	// performance measurement vars

	float		framerate, frametime;
	int		frames_per_second,
			samples_per_second;
	char		perf_txt[30];

	char		cursor_txt[60];
	float		cursor_textpos[2];
	bool		do_cursor, do_stats;
	float		cursor[3], cursor_inc;
	bool		shift_key_pressed;
} Lpanel_t; // end Lpanel_t

extern Lpanel_t		*Lpanel_new(void);
extern void		Lpanel_delete(Lpanel_t *p);
extern void		Lpanel_init(Lpanel_t *p);
extern void		Lpanel_fini(Lpanel_t *p);

// internal functions

extern void		Lpanel_genGraphicsData(Lpanel_t *p);
extern void		Lpanel_growLights(Lpanel_t *p);
extern void		Lpanel_growObjects(Lpanel_t *p);
extern void		Lpanel_growAlphaObjects(Lpanel_t *p);
extern void		Lpanel_growSwitches(Lpanel_t *p);
extern lpObject_t	*Lpanel_addObject(Lpanel_t *p);
extern void		Lpanel_addAlphaObject(Lpanel_t *p, lpObject_t *obj);
extern int		Lpanel_pick(Lpanel_t *p, int button, int state,	// mouse pick function
				    int x, int y);
extern void		Lpanel_setProjection(Lpanel_t *p, bool dopick);
extern void		Lpanel_setModelview(Lpanel_t *p, bool dopick);

// public funcions

extern lpSwitch_t	*Lpanel_findSwitchByName(Lpanel_t *p, char *name);
extern int		Lpanel_test(Lpanel_t *p, int n);	// general test function
extern int		Lpanel_addLight(Lpanel_t *p, const char *name, lp_obj_parm_t *obj,
					const char *buff);
extern int		Lpanel_addLightToGroup(Lpanel_t *p, int lightnum, int groupnum);

extern int		Lpanel_addSwitch(Lpanel_t *p, const char *name, lp_obj_parm_t *obj,
					 const char *buff);
extern bool		Lpanel_addSwitchCallback(Lpanel_t *p, const char *name,
						 lp_switch_cbf_t cbfunc, int userval);
extern void		Lpanel_addQuitCallback(Lpanel_t *p, lp_quit_cbf_t cbfunc);

extern void		Lpanel_bindSimclock(Lpanel_t *p, uint64_t *addr);
extern void		Lpanel_bindRunFlag(Lpanel_t *p, uint8_t *addr);

extern void		Lpanel_draw(Lpanel_t *p);
extern struct lpLight	*Lpanel_findLightByName(Lpanel_t *p, char *name);
extern lpObject_t	*Lpanel_findObjectByName(Lpanel_t *p, char *name);

extern void		Lpanel_framerate_set(Lpanel_t *p, float n);
extern void		Lpanel_ignoreBindErrors(Lpanel_t *p, bool f);
extern void		Lpanel_printLights(Lpanel_t *p);
extern bool		Lpanel_readConfig(Lpanel_t *p, const char *fname);
extern void		Lpanel_sampleData(Lpanel_t *p);
extern void		Lpanel_sampleDataWarp(Lpanel_t *p, int clockwarp);
extern void		Lpanel_sampleLightGroup(Lpanel_t *p, int groupnum, int clockval);
extern void		Lpanel_sampleSwitches(Lpanel_t *p);
extern void		Lpanel_setConfigRootPath(Lpanel_t *p, const char *path);
extern bool		Lpanel_bindLight8(Lpanel_t *p, const char *name, void *loc,
					  int start_bit_number);
extern bool		Lpanel_bindLight16(Lpanel_t *p, const char *name, void *loc,
					   int start_bit_number);
extern bool		Lpanel_bindLightfv(Lpanel_t *p, const char *name, void *loc);
extern bool		Lpanel_bindLight32(Lpanel_t *p, const char *name, void *loc,
					   int start_bit_number);
extern bool		Lpanel_bindLight64(Lpanel_t *p, const char *name, void *loc,
					   int start_bit_number);

extern bool		Lpanel_bindLight8invert(Lpanel_t *p, const char *name, void *loc,
						int start_bit_number, uint8_t mask);
extern bool		Lpanel_bindLight16invert(Lpanel_t *p, const char *name, void *loc,
						 int start_bit_number, uint16_t mask);
extern bool		Lpanel_bindLight32invert(Lpanel_t *p, const char *name, void *loc,
						 int start_bit_number, uint32_t mask);
extern bool		Lpanel_bindLight64invert(Lpanel_t *p, const char *name, void *loc,
						 int start_bit_number, uint64_t mask);

extern bool		Lpanel_bindSwitch8(Lpanel_t *p, const char *name, void *loc_down,
					   void *loc_up, int start_bit_number);
extern bool		Lpanel_bindSwitch16(Lpanel_t *p, const char *name, void *loc_down,
					    void *loc_up, int start_bit_number);
extern bool		Lpanel_bindSwitch32(Lpanel_t *p, const char *name, void *loc_down,
					    void *loc_up, int start_bit_number);
extern bool		Lpanel_bindSwitch64(Lpanel_t *p, const char *name, void *loc_down,
					    void *loc_up, int start_bit_number);
extern bool		Lpanel_smoothLight(Lpanel_t *p, const char *name, int nframes);

// window functions

extern int		Lpanel_openWindow(Lpanel_t *p, const char *title);
extern void		Lpanel_destroyWindow(Lpanel_t *p);
extern void		Lpanel_doPickProjection(Lpanel_t *p);
extern void		Lpanel_doPickModelview(Lpanel_t *p);

#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
extern LRESULT CALLBACK	Lpanel_WndProc(Lpanel_t *p, UINT Msg, WPARAM wParam, LPARAM lParam);
#else
extern void		Lpanel_resizeWindow(Lpanel_t *p);
#endif

extern void		Lpanel_initGraphics(Lpanel_t *p);
#ifdef WANT_SDL
extern void		Lpanel_procEvent(Lpanel_t *p, SDL_Event *event);
#else
extern void		Lpanel_procEvents(Lpanel_t *p);
#endif
extern void		Lpanel_resolveObjectInstances(Lpanel_t *p);

// dev functions

extern void		Lpanel_draw_cursor(Lpanel_t *p);
extern void		Lpanel_inc_cursor(Lpanel_t *p, float x, float y);
extern void		Lpanel_make_cursor_text(Lpanel_t *p);

extern void		Lpanel_draw_stats(Lpanel_t *p);

// class lpLight
// -------------

typedef	void (*lp_light_df_t)(struct lpLight *p);	// light draw function pointer
typedef	void (*lp_light_sdf_t)(struct lpLight *p);	// light sample data function pointer

typedef struct lpLight {
	Lpanel_t	*panel;

	int		bindtype;	// bound to bit or array of floats

	int		smoothing;	// 0= no intensity transition smoothing
					// >0 = number of frames to transition.

	uint64_t	*simclock;
	int		*clock_warp;

	uint8_t		default_runflag,
			*runflag;

	char		*name;
	void		*dataptr;	// pointer to data to sample
	int		datatype;	// datatype dataptr points to
	int		bitnum;		// bit in data controlling this light

	char		*obj_refname;	// name of object if this light references one.
	lpObject_t	*obj_ref;	// pointer to object if this light references one.

	uint64_t	t1, t2,
			on_time;

	uint64_t	start_clock,
			old_clock;

	float		intensity,
			intense_samples[2],	// for smoothing if enabled
			intense_incr,		// increment value for smoothing transitions
			intense_curr;		// current intensity for smoothing
	int 		intense_curr_idx;	// index for intense_samples[]

	float		color[3];	// adjusted color
	unsigned char	state,
			old_state;
	bool		dirty;

	lp_obj_parm_t	*parms;

	lp_light_df_t	drawFunc;
	lp_light_sdf_t	sampleDataFunc;
} lpLight_t;

extern lpLight_t	*lpLight_new(void);
extern void		lpLight_delete(lpLight_t *p);
extern void		lpLight_init(lpLight_t *p);
extern void		lpLight_fini(lpLight_t *p);

extern void		lpLight_bindRunFlag(lpLight_t *p, uint8_t *addr);

extern void		lpLight_bindData8(lpLight_t *p, uint8_t *ptr);
extern void		lpLight_bindData8invert(lpLight_t *p, uint8_t *ptr);

extern void		lpLight_bindData16(lpLight_t *p, uint16_t *ptr);
extern void		lpLight_bindDatafv(lpLight_t *p, float *ptr);

extern void		lpLight_bindData16invert(lpLight_t *p, uint16_t *ptr);
extern void		lpLight_bindData32(lpLight_t *p, uint32_t *ptr);
extern void		lpLight_bindData32invert(lpLight_t *p, uint32_t *ptr);
extern void		lpLight_bindData64(lpLight_t *p, uint64_t *ptr);
extern void		lpLight_bindData64invert(lpLight_t *p, uint64_t *ptr);

extern void		lpLight_bindSimclock(lpLight_t *p, uint64_t *addr, int *clockwarp);

extern void		lpLight_calcIntensity(lpLight_t *p);
extern void		lpLight_draw(lpLight_t *p);
extern void		lpLight_print(lpLight_t *p);

extern void		lpLight_setupData(lpLight_t *p);
extern void		lpLight_sampleData(lpLight_t *p);

extern void		lpLight_setName(lpLight_t *p, const char *name);
extern void		lpLight_setBitNumber(lpLight_t *p, int bitnum);

#endif /* !_LPANEL_DEFS */
