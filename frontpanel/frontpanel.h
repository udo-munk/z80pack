// frontpanel.h	frontpanel api include file

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

#ifndef _FRONTPANEL_DEFS
#define _FRONTPANEL_DEFS

#include <stdbool.h>
#include <stdint.h>
#ifdef WANT_SDL
#include <SDL.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define FP_SW_DOWN	0
#define FP_SW_UP	1
#define FP_SW_CENTER	2

extern int	fp_test(int n);
extern int	fp_init2(const char *cfg_root_path, const char *cfg_fname, int size);
extern int	fp_init(const char *cfg_fname);
extern void	fp_openWindow(void);
#ifdef WANT_SDL
extern void	fp_procEvent(SDL_Event *event);
extern void	fp_draw(bool tick);
#else
extern void	fp_procEvents(void);
extern void	fp_draw(void);
#endif
extern void	fp_framerate(float f);
extern void	fp_sampleData(void);
extern void	fp_sampleDataWarp(int clockwarp);
extern void	fp_sampleLightGroup(int groupnum, int clockwarp);
extern void	fp_sampleSwitches(void);
extern void	fp_quit(void);

/* data binding functions */

extern void 	fp_bindSimclock(uint64_t *addr);
extern void	fp_bindRunFlag(uint8_t *addr);

extern int	fp_bindLight64(const char *name, uint64_t *bits, int bitnum);
extern int	fp_bindLight32(const char *name, uint32_t *bits, int bitnum);
extern int	fp_bindLight16(const char *name, uint16_t *bits, int bitnum);
extern int	fp_bindLightfv(const char *name, float *bits);
extern int	fp_bindLight8(const char *name, uint8_t *bits, int bitnum);
extern int	fp_bindLight8invert(const char *name, uint8_t *bits, int bitnum, uint8_t mask);
extern int	fp_bindLight16invert(const char *name, uint16_t *bits, int bitnum, uint16_t mask);
extern int	fp_bindLight32invert(const char *name, uint32_t *bits, int bitnum, uint32_t mask);
extern int	fp_bindLight64invert(const char *name, uint64_t *bits, int bitnum, uint64_t mask);

extern int	fp_bindSwitch64(const char *name, uint64_t *loc_down, uint64_t *loc_up,
				int bitnum);
extern int	fp_bindSwitch32(const char *name, uint32_t *loc_down, uint32_t *loc_up,
				int bitnum);
extern int	fp_bindSwitch16(const char *name, uint16_t *loc_down, uint16_t *loc_up,
				int bitnum);
extern int	fp_bindSwitch8(const char *name, uint8_t *loc_down, uint8_t *loc_up, int bitnum);

extern int	fp_smoothLight(const char *name, int nframes);

/* callbacks */

extern int	fp_addSwitchCallback(const char *name, void (*cbfunc)(int state, int val),
				     int userval);
extern void	fp_addQuitCallback(void (*cbfunc)(void));

/* error reporting */

extern void	fp_ignoreBindErrors(int n);	/* n=1 ignore, n=0 report errors */

#ifdef __cplusplus
}
#endif

#endif /* !_FRONTPANEL_DEFS */
