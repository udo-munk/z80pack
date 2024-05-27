// lpanel.h  lightpanel


/* Copyright (c) 2007-2008, John Kichury

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

#ifndef __LPANEL_DEFS
#define __LPANEL_DEFS


#include <stdio.h>
#if defined (__MINGW32__) || defined (_WIN32) || defined (_WIN32_) || defined (__WIN32__)
#include <GL/gl.h>
#include <windows.h>
#else
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif


#define LP_MAX_LIGHT_GROUPS 10


// forward references

class lpLight;
class lpSwitch;
class Lpanel;


enum obj_types { LP_NULL, LP_LED };
enum obj_subtypes { LP_SUBTYPE_NULL, LP_LED_3D, LP_LED_2D };
enum gfx_projection { LP_ORTHO, LP_PERSPECTIVE };

// light sampling datatypes

enum light_bindtypes { 
    LBINDTYPE_NULL, 
    LBINDTYPE_BIT, 		// data pointer points to a single word of bits (bitnum is used as a bit number)
    LBINDTYPE_FLOATV  	// data pointer points to an array (bitnum is used as an index)
    };

// view structure
//

typedef struct
{
  int	 projection,		// ortho, perspective
	 do_depthtest;		// true = enable depth test
  float  rot[3],
	 pan[3];
  double aspect, fovy, znear, zfar;

  int	 redo_projections;

} view_t;

// object parm structure 
// ---------------------

typedef struct
{
  int	type,		// LP_LIGHT, LP_SWITCH etc.
	subtype,	// LP_LED
	group;

  float	pos[3],		// xyz position
	color[3],	// rgb color
	scale[3];
} lp_obj_parm_t;


typedef struct
{
  int 	num_items,
      	max_items,
	*list;
  
} lp_light_group;


#include "lp_gfx.h"
#include "lp_switch.h"

#if defined (__MINGW32__) || defined (_WIN32) || defined (_WIN32_) || defined (__WIN32__)
static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif

// light panel class
// -----------------

class Lpanel
{

 private:

  int		num_lights,
		max_lights;

  lpLight	**lights;

  lp_light_group light_groups[LP_MAX_LIGHT_GROUPS];

  lpSwitch	**switches;

  int		num_switches,
		max_switches;

#if defined (__MINGW32__) || defined (_WIN32) || defined (_WIN32_) || defined (__WIN32__)
  HINSTANCE hInstance;
  HWND hWnd;
  WNDCLASSEX wc;
  HDC hDC;
  HGLRC hRC;
  PIXELFORMATDESCRIPTOR pfd;
#else
  Display	*dpy;		// Xwindows display
  Window	window;		// Xwindows window
  XVisualInfo	*vi;
  GLXContext	cx;
  Atom		wmDeleteMessage; // for processing window close event
#endif

  view_t	view;

  lpElement	*curr_element;
  vertex_t	*curr_vertex;
  int		envmap_detected;

  char		*config_root_path;	// pathname to configuration root (default is ./)

  // internal methods

  int 		parseConfigParms(const char *buff, lp_obj_parm_t *obj);
  void 		genGraphicsData(void);
  void 		growLights(void);
  void 		growObjects(void);
  void 		growAlphaObjects(void);
  void 		growSwitches(void);
  lpObject 	*addObject(void);
  void		addAlphaObject(lpObject *p);
  lpTextures	textures;
  int		pick(int button, int state, int x, int y);	// mouse pick function
  void		setProjection(int forpick);
  void		setModelview(int eye);

 public:

  Lpanel(void);
  ~Lpanel(void);

  uint64_t	default_clock,
  		*simclock,
		old_clock;
  int		clock_warp;
  int		ignore_bind_errors;
  uint8_t	default_runflag,
  		*runflag;
  lpSwitch	*mom_switch_pressed,
		*findSwitchByName(char *name);

  float		lightcolor[3],	// default light color
		lightsize[3];	// default light size

  void		(*quit_callbackfunc)(void);

  int test(int n);	// general test function

  int addLight(const char *name, lp_obj_parm_t *obj, const char *buff);
  int addLightToGroup( int lightnum, int groupnum);

  int addSwitch(const char *name, lp_obj_parm_t *obj, const char *buff, Lpanel *p);
  int addSwitchCallback(const char *name, void (*cbfunc)(int state, int val), int userval);
  void addQuitCallback(void (*cbfunc)(void));

//  void bindSimclock(const uint64_t *addr);
//  void bindRunFlag(const uint8_t *addr);

  void bindSimclock(uint64_t *addr);
  void bindRunFlag(uint8_t *addr);

  void draw(void);
  lpLight *findLightByName(char *name);
  lpObject *findObjectByName(char *name);

  void framerate_set(float n) { framerate = n; frametime=1./n; };
  void	ignoreBindErrors(int n) { ignore_bind_errors = n; };
  void printLights(void);
  void Quit(void);
  int  readConfig(const char *fname);
  //int  readConfig(const char *cfg_rootpath, const char *fname);
  void sampleData(void);
  void sampleDataWarp(int clockwarp);
  void sampleLightGroup(int groupnum, int clockval);
  void sampleSwitches(void);
  void setConfigRootPath(const char *path);
  int bindLight8(const char *name, void *loc, int start_bit_number);
  int bindLight16(const char *name, void *loc, int start_bit_number);
  int bindLightfv(const char *name, void *loc);
  int bindLight32(const char *name, void *loc, int start_bit_number);
  int bindLight64(const char *name, void *loc, int start_bit_number);

  int bindLight8invert(const char *name, void *loc, int start_bit_number, uint8_t mask);
  int bindLight16invert(const char *name, void *loc, int start_bit_number, uint16_t mask);
  int bindLight32invert(const char *name, void *loc, int start_bit_number, uint32_t mask);
  int bindLight64invert(const char *name, void *loc, int start_bit_number, uint64_t mask);

  int bindSwitch8(const char *name, void *loc_down, void *loc_up, int start_bit_number);
  int bindSwitch16(const char *name, void *loc_down, void *loc_up, int start_bit_number);
  int bindSwitch32(const char *name, void *loc_down, void *loc_up, int start_bit_number);
  int bindSwitch64(const char *name, void *loc_down, void *loc_up, int start_bit_number);
  int smoothLight(const char *name, int nframes);

  int		num_objects,
		max_objects,
		num_alpha_objects,
		max_alpha_objects;

  lpObject	**objects,
		*curr_object,
		**alpha_objects;

  lpBBox	bbox;

  // window methods

  int  openWindow(const char *title);
  void destroyWindow(void);
  void doPickProjection(void);
  void doPickModelview(void);

#if defined (__MINGW32__) || defined (_WIN32) || defined (_WIN32_) || defined (__WIN32__)

  LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

#else
  void resizeWindow(void);
#endif

  void initGraphics();
  void procEvents(void);
  void resolveObjectInstances(void);

  GLint viewport[4];
  int   window_xsize,
	window_ysize;

  // dev vars and methods

  // performance measurement vars

  float		framerate, frametime;
  int		frames_per_second,
		samples_per_second;
  char		perf_txt[30];

  char		cursor_txt[60];
  float		cursor_textpos[2];
  int		do_cursor, do_stats;
  float		cursor[3], cursor_inc;
  int		shift_key_pressed;
  void		draw_cursor(void);
  void		inc_cursor(float x, float y);
  void		make_cursor_text(void);

  void		draw_stats(void);

}; // end Lpanel



// class lpLight
// -------------


class lpLight
{
  private:


  public:

  lpLight(void);
  ~lpLight(void);

  Lpanel *panel;

  int	 bindtype;	// bound to bit or array of floats

  int	 smoothing;	// 0= no intensity transition smoothing, >0 = number of frames to transition.

  uint64_t *simclock;
  int	 *clock_warp;

  uint8_t default_runflag,
  	 *runflag;
  //void bindRunFlag(const uint8_t *addr);
  void bindRunFlag(uint8_t *addr);

  char *name;
  void *dataptr;	// pointer to data to sample
  int   datatype;	// datatype dataptr points to
  int   bitnum;		// bit in data controlling this light

  char *obj_refname;	// name of object if this light references one.
  lpObject *obj_ref;	// pointer to object if this light references one.

#if 0
  unsigned long t1, t2,
		on_time;
#endif
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
		old_state,
		dirty;

  lp_obj_parm_t *parms;

  void bindData8(uint8_t *ptr);
  void bindData8invert(uint8_t *ptr);

  void bindData16(uint16_t *ptr);
  void bindDatafv(float *ptr);

  void bindData16invert(uint16_t *ptr);
  void bindData32(uint32_t *ptr);
  void bindData32invert(uint32_t *ptr);
  void bindData64(uint64_t *ptr);
  void bindData64invert(uint64_t *ptr);

  void bindSimclock(uint64_t *addr, int *clockwarp);

  void calcIntensity(void);
  void draw(void);
  void (*drawFunc)(lpLight *lp);
  void print(void);

  void setupData(void);
  void (*sampleDataFunc)(lpLight *p);
  void sampleData(void);

  void setName(const char *name);
  void setBitNumber(int _bitnum) { bitnum = _bitnum; };
};



#endif
