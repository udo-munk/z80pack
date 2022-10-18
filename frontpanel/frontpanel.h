/* frontpanel.h		front panel api include file */


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

#ifndef _FP_API_DEFS
#define _FP_API_DEFS

#ifdef __cplusplus
 extern "C" {
#endif


#ifndef _SIM_DEFS_H_
#define _SIM_DEFS_H_
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
#endif
typedef unsigned long long uint64;

#define FP_SW_DOWN	0
#define FP_SW_UP	1
#define FP_SW_CENTER	2

int	fp_test(int n);
int     fp_init2(const char *cfg_root_path, const char *cfg_fname, int size);
int     fp_init(const char *cfg_fname);
int	fp_openWindow(const char *title);
void	fp_framerate(float f);
void	fp_sampleData(void);
void	fp_sampleDataWarp(int clockwarp);
void	fp_sampleLightGroup(int groupnum, int clockwarp);
void	fp_sampleSwitches(void);
void	fp_quit(void);

/* data binding functions */

//void 	fp_bindSimclock(const uint64 *addr);
////void	fp_bindRunFlag(const uint8 *addr);

void 	fp_bindSimclock(uint64 *addr);
void	fp_bindRunFlag(uint8 *addr);

int	fp_bindLight64(const char *name, uint64 *bits, int bitnum);
int	fp_bindLight32(const char *name, uint32 *bits, int bitnum);
int	fp_bindLight16(const char *name, uint16 *bits, int bitnum);
int	fp_bindLightfv(const char *name, float *bits);
int	fp_bindLight8(const char *name, uint8 *bits, int bitnum);
int	fp_bindLight8invert(const char *name, uint8 *bits, int bitnum, uint8 mask);
int	fp_bindLight16invert(const char *name, uint16 *bits, int bitnum, uint16 mask);
int	fp_bindLight32invert(const char *name, uint32 *bits, int bitnum, uint32 mask);
int	fp_bindLight64invert(const char *name, uint64 *bits, int bitnum, uint64 mask);

int	fp_bindSwitch64(const char *name, uint64 *loc_down, uint64*loc_up, int bitnum);
int	fp_bindSwitch32(const char *name, uint32 *loc_down, uint32 *loc_up, int bitnum);
int	fp_bindSwitch16(const char *name, uint16 *loc_down, uint16 *loc_up, int bitnum);
int	fp_bindSwitch8(const char *name, uint8 *loc_down, uint8 *loc_up, int bitnum);


int	fp_smoothLight(const char *name, int nframes);

/* callbacks */

int	fp_addSwitchCallback(const char *name, void (*cbfunc)(int state, int val), int userval);
void	fp_addQuitCallback(void (*cbfunc)(void));

/* error reporting */

void    fp_ignoreBindErrors(int n);		/* n=1 - ignore    n=0 report errors */
#ifdef __cplusplus
 }
#endif



#endif

