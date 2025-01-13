// lp_switch.h	lightpanel switch class

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

#ifndef _LP_SWITCH_DEFS
#define _LP_SWITCH_DEFS

#include <stdint.h>
#ifdef WANT_SDL
#include <SDL_mixer.h>
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#endif

#include "lpanel.h"

enum lp_switch_states {
	LP_SWITCH_DOWN = 0,		// switch is down
	LP_SWITCH_UP = 1,		// switch is up
	LP_SWITCH_CENTER = 2		// switch is centered (for momentary MOM_OFF_MOM)
};

enum lp_switch_operation {
	LP_SWITCH_OP_ON_OFF = 0,	// on/off operation
	LP_SWITCH_OP_MOM_OFF_MOM = 1,	// momentary down, centered, momentary up
	LP_SWITCH_OP_OFF_MOM = 2	// centered (down), momentary up
};

enum lp_switch_gfx_modes {
	LP_SWITCH_GFX_TOGGLE = 0,	// uses internal toggle switch graphics
	LP_SWITCH_GFX_PADDLE = 1,	// uses internal paddle switch graphics
	LP_SWITCH_GFX_OBJECT_REF = 2	// uses a graphics 'object' from the config file
};

enum lp_switch_datatypes {
	LP_SWITCH_DATATYPE_UINT8 = 8,
	LP_SWITCH_DATATYPE_UINT16 = 16,
	LP_SWITCH_DATATYPE_UINT32 = 32,
	LP_SWITCH_DATATYPE_UINT64 = 64
};

// High order bit in the OpenGL select name used to detect
// the switch 'up_target' or 'down_target' being clicked with the mouse.
// The low order 31 bits are used to store the switch number (index within the array of
// switches). The high order bit indicates the target (1=up 0=down).

#define LP_SW_PICK_UP_BIT 0x80000000
#define LP_SW_PICK_IDMASK 0x0fffffff

typedef void (*lp_switch_cbf_t)(int state, int val);	// switch callback function pointer
typedef void (*lp_switch_df_t)(struct lpSwitch *swtch);	// switch draw function pointer

typedef struct lpSwitch {
	char		*name;

	int		type;			// toggle/paddle/object_ref
	uint8_t		state,			// 0=off 1=on 2=center(for mom-off-mom switches)
			gfx_mode,		// one of lp_switch_gfx_modes
			operation,		// one of lp_switch_operation
			num_object_refs;	// number of object references

	void		*dataptr[2];		// pointer to data for up and down position
	int		datatype;		// datatype dataptr points to
	int		bitnum;			// bit in data controlling this light
	lp_switch_cbf_t	callback;		// user callback
	int		userdata;

	char		**object_ref_names;
	lpObject_t	*object_refs[3];	// object references for gfx_mode=object

	lp_obj_parm_t	*parms;
	struct Lpanel	*panel;

	// rotation values for 3 switch states (for 3D models)

	float		rotate[3][4];		// ang, 3D unit vector for 3 switch states

	GLuint		select_up_name,
			select_dn_name;

	float		up_target[4][3],	// mouse pick targets for activation
			down_target[4][3];

	lp_switch_df_t	drawFunc;

#ifdef WANT_SDL
	Mix_Chunk	*on_sound;
	Mix_Chunk	*off_sound;
#endif
} lpSwitch_t;

extern lpSwitch_t	*lpSwitch_new(void);
extern void		lpSwitch_delete(lpSwitch_t *p);
extern void		lpSwitch_init(lpSwitch_t *p);
extern void		lpSwitch_fini(lpSwitch_t *p);

extern void		lpSwitch_action(lpSwitch_t *p, int val);
extern void		lpSwitch_addCallback(lpSwitch_t *p, lp_switch_cbf_t cbfunc, int userval);
extern void		lpSwitch_bindData64(lpSwitch_t *p, uint64_t *ptr_down, uint64_t *ptr_up,
					    int bit_number);
extern void		lpSwitch_bindData32(lpSwitch_t *p, uint32_t *ptr_down, uint32_t *ptr_up,
					    int bit_number);
extern void		lpSwitch_bindData16(lpSwitch_t *p, uint16_t *ptr_down, uint16_t *ptr_up,
					    int bit_number);
extern void		lpSwitch_bindData8(lpSwitch_t *p, uint8_t *ptr_down, uint8_t *ptr_up,
					   int bit_number);
extern void		lpSwitch_draw(lpSwitch_t *p);
extern void		lpSwitch_drawForPick(lpSwitch_t *p);
extern void		lpSwitch_sampleData(lpSwitch_t *p);

extern void		lpSwitch_setName(lpSwitch_t *p, const char *name);
extern void		lpSwitch_setupData(lpSwitch_t *p, int sw_num); // resolve gfx refs etc.

#endif /* !_LP_SWITCH_DEFS */
